#pragma once

#include <Arduino.h>

// Цифровые входы DI1-DI8
#define PIN_DI1 4
#define PIN_DI2 5
#define PIN_DI3 6
#define PIN_DI4 7
#define PIN_DI5 8
#define PIN_DI6 9
#define PIN_DI7 10
#define PIN_DI8 11

// I2C, реле RO1-RO8 подключены через расширитель PCA9554
#define PIN_I2C_SDA 42
#define PIN_I2C_SCL 41
#define PCA9554_ADDR 0x20

// Аналоговый выход (ШИМ) — тестовый, для проверки при монтаже
#define PIN_ANALOG_OUT 21
#define PERIPH_ANALOG_FREQ_HZ 5000
#define PERIPH_ANALOG_RESOLUTION_BITS 8

// Ethernet W5500 (SPI) — нужен для работы веб-интерфейса
#define PIN_ETH_MOSI 13
#define PIN_ETH_MISO 14
#define PIN_ETH_SCK 15
#define PIN_ETH_CS 16
#define PIN_ETH_INT 12
#define PIN_ETH_RST -1

#define ETH_MAC_ADDR \
  { 0x02, 0x00, 0x00, 0x45, 0x53, 0x01 }
#define ETH_FALLBACK_IP 192, 168, 1, 200

#define PERIPH_INPUT_COUNT 8
#define PERIPH_RELAY_COUNT 8

void Peripherals_Init();
void Peripherals_Loop();

bool Peripherals_ReadInput(uint8_t index);
uint8_t Peripherals_ReadAllInputs();

void Peripherals_SetRelay(uint8_t index, bool state);
bool Peripherals_GetRelay(uint8_t index);
void Peripherals_SetAllRelays(uint8_t mask);
uint8_t Peripherals_GetAllRelays();
bool Peripherals_RelaysAvailable();

void Peripherals_SetAnalogOut(uint8_t value);
uint8_t Peripherals_GetAnalogOut();

bool Peripherals_EthAvailable();
bool Peripherals_EthLinkUp();
IPAddress Peripherals_EthGetIp();
String Peripherals_EthGetMac();

// Режим теста: страница /io шлёт пинг (GET /api/status) раз в 2 сек, пока открыта —
// выставляется прямым присваиванием оттуда, где нужно (сейчас — web.cpp).
// true — выходы под ручным управлением с веб-страницы, логика (logic.cpp) не должна
// их трогать. Если пинга долго нет — считаем, что со страницы ушли, и сами сбрасываем
// флаг в Peripherals_Loop() по таймауту.
#define PERIPH_TEST_MODE_TIMEOUT_MS 5000
extern bool testModeActive;
extern unsigned long testModeLastPingMs;

// Обработка "удержания" входа сверх аппаратного дребезга (см. updateInputs() в
// peripherals.cpp) — например, кнопка должна быть зажата дольше debounceMs, чтобы
// засчиталось нажатие. Общая утилита для входов, поэтому здесь, а не в logic.cpp.
// Это фиксированная константа железа, не пользовательская настройка — в NVS не идёт.
#define PERIPH_BUTTON_DEBOUNCE_MS 200

struct DebouncedButton {
  bool wasPressed = false;
  unsigned long pressStartMs = 0;
  bool actionFired = false;
};

// Возвращает true один раз за нажатие — когда pressed удержан дольше debounceMs.
bool Peripherals_ButtonHeld(DebouncedButton &btn, bool pressed, unsigned long debounceMs);
