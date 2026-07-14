#include "peripherals.h"

#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>

#include "dhcp_server.h"
#include "pca9554.h"

static const uint8_t inputPins[PERIPH_INPUT_COUNT] = {
    PIN_DI1, PIN_DI2, PIN_DI3, PIN_DI4, PIN_DI5, PIN_DI6, PIN_DI7, PIN_DI8};

// DI-каналы на этой плате — opto-isolated, "sinking": в покое вход подтянут
// внутренней подтяжкой ESP32 к HIGH, при срабатывании оптопара тянет линию
// в LOW (см. официальный конфиг ESPHome для этой платы: INPUT_PULLUP + inverted).
#define PERIPH_INPUT_DEBOUNCE_MS 10

static bool inputRawState[PERIPH_INPUT_COUNT] = {false};
static bool inputStableState[PERIPH_INPUT_COUNT] = {false};
static unsigned long inputChangeTime[PERIPH_INPUT_COUNT] = {0};

static Pca9554 relays(PCA9554_ADDR);
static byte ethMac[] = ETH_MAC_ADDR;

static DhcpServer ethDhcpServer;
static bool ethDhcpServerActive = false;
static bool ethAvailable = false;
static bool relaysAvailable = false;
static uint8_t analogOutValue = 0;

bool testModeActive = false;
unsigned long testModeLastPingMs = 0;

static void ethInit() {
  SPI.begin(PIN_ETH_SCK, PIN_ETH_MISO, PIN_ETH_MOSI, PIN_ETH_CS);
  Ethernet.init(PIN_ETH_CS);

  // Дефолтный таймаут DHCP в этой библиотеке — 60 секунд, что делает загрузку
  // невыносимо долгой, если кабель не воткнут или воткнут не в роутер.
  // Сокращаем до ~4 секунд — обычному роутеру этого с запасом хватает,
  // а если ответа нет, быстрее уходим в резервный статический IP ниже.
  bool dhcpOk = Ethernet.begin(ethMac, 5000, 1000) != 0;

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    // Чип W5500 не откликнулся на SPI — работаем без проводного порта, только по Wi-Fi.
    ethAvailable = false;
    return;
  }
  ethAvailable = true;

  if (!dhcpOk) {
    // Внешний DHCP не ответил (скорее всего кабель воткнут напрямую в ПК,
    // без роутера) — поднимаем свой DHCP-сервер на статическом IP.
    IPAddress fallbackIp(ETH_FALLBACK_IP);
    Ethernet.begin(ethMac, fallbackIp);

    IPAddress offeredIp = fallbackIp;
    offeredIp[3] += 1;

    ethDhcpServer.begin(fallbackIp, offeredIp, IPAddress(255, 255, 255, 0));
    ethDhcpServerActive = true;
  }
}

void Peripherals_Init() {
  for (uint8_t i = 0; i < PERIPH_INPUT_COUNT; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
    bool active = digitalRead(inputPins[i]) == LOW;
    inputRawState[i] = active;
    inputStableState[i] = active;
  }

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  relaysAvailable = relays.isPresent();
  if (relaysAvailable) {
    relays.begin(0x00);
  }

  ledcAttach(PIN_ANALOG_OUT, PERIPH_ANALOG_FREQ_HZ, PERIPH_ANALOG_RESOLUTION_BITS);
  ledcWrite(PIN_ANALOG_OUT, 0);

  ethInit();
}

static void updateInputs() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < PERIPH_INPUT_COUNT; i++) {
    bool active = digitalRead(inputPins[i]) == LOW;
    if (active != inputRawState[i]) {
      inputRawState[i] = active;
      inputChangeTime[i] = now;
    } else if (active != inputStableState[i] && now - inputChangeTime[i] >= PERIPH_INPUT_DEBOUNCE_MS) {
      inputStableState[i] = active;
    }
  }
}

bool Peripherals_ButtonHeld(DebouncedButton &btn, bool pressed, unsigned long debounceMs) {
  if (pressed && !btn.wasPressed) {
    btn.pressStartMs = millis();
    btn.actionFired = false;
  }

  bool fire = false;
  if (pressed && !btn.actionFired && millis() - btn.pressStartMs >= debounceMs) {
    btn.actionFired = true;
    fire = true;
  }

  btn.wasPressed = pressed;
  return fire;
}

void Peripherals_Loop() {
  updateInputs();

  if (testModeActive && millis() - testModeLastPingMs > PERIPH_TEST_MODE_TIMEOUT_MS) {
    testModeActive = false;
  }

  if (ethDhcpServerActive) {
    ethDhcpServer.loop();
  }
}

bool Peripherals_ReadInput(uint8_t index) {
  if (index >= PERIPH_INPUT_COUNT) return false;
  return inputStableState[index];
}

uint8_t Peripherals_ReadAllInputs() {
  uint8_t mask = 0;
  for (uint8_t i = 0; i < PERIPH_INPUT_COUNT; i++) {
    if (Peripherals_ReadInput(i)) mask |= (1 << i);
  }
  return mask;
}

void Peripherals_SetRelay(uint8_t index, bool state) {
  if (!relaysAvailable || index >= PERIPH_RELAY_COUNT) return;
  relays.writePin(index, state);
}

bool Peripherals_GetRelay(uint8_t index) {
  if (!relaysAvailable || index >= PERIPH_RELAY_COUNT) return false;
  return relays.readAll() & (1 << index);
}

void Peripherals_SetAllRelays(uint8_t mask) {
  if (!relaysAvailable) return;
  relays.writeAll(mask);
}

uint8_t Peripherals_GetAllRelays() {
  if (!relaysAvailable) return 0;
  return relays.readAll();
}

bool Peripherals_RelaysAvailable() {
  return relaysAvailable;
}

void Peripherals_SetAnalogOut(uint8_t value) {
  analogOutValue = value;
  ledcWrite(PIN_ANALOG_OUT, value);
}

uint8_t Peripherals_GetAnalogOut() {
  return analogOutValue;
}

bool Peripherals_EthAvailable() {
  return ethAvailable;
}

bool Peripherals_EthLinkUp() {
  return Ethernet.linkStatus() == LinkON;
}

IPAddress Peripherals_EthGetIp() {
  return Ethernet.localIP();
}
