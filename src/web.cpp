#include "web.h"

#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <Ethernet.h>
#include <HTTPUpdate.h>
#include <Update.h>
#include <WiFi.h>

#include "logic.h"
#include "peripherals.h"
#include "settings.h"
#include "wifi_client.h"

static EthernetServer ethServer(WEB_SERVER_PORT);
static WiFiServer apServer(WEB_SERVER_PORT);
static DNSServer dnsServer;

static String sessionToken = "";

// -2 = ошибка, -1 = не запущено, 0-99 = идёт скачивание, 100 = готово
static volatile int updateProgressPercent = -1;

static String urlDecode(const String &input) {
  String output;
  output.reserve(input.length());
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c == '+') {
      output += ' ';
    } else if (c == '%' && i + 2 < input.length()) {
      char hex[3] = {input[i + 1], input[i + 2], 0};
      output += (char)strtol(hex, nullptr, 16);
      i += 2;
    } else {
      output += c;
    }
  }
  return output;
}

static String getFormField(const String &body, const String &key) {
  int pos = body.indexOf(key + "=");
  if (pos < 0) return "";
  int start = pos + key.length() + 1;
  int end = body.indexOf('&', start);
  if (end < 0) end = body.length();
  return urlDecode(body.substring(start, end));
}

static String extractCookieValue(const String &cookieHeader, const String &key) {
  int pos = cookieHeader.indexOf(key + "=");
  if (pos < 0) return "";
  int start = pos + key.length() + 1;
  int end = cookieHeader.indexOf(';', start);
  if (end < 0) end = cookieHeader.length();
  return cookieHeader.substring(start, end);
}

static bool isAuthorized(const String &cookieHeader) {
  if (sessionToken.length() == 0) return false;
  return extractCookieValue(cookieHeader, SESSION_COOKIE_NAME) == sessionToken;
}

static String jsonEscape(const String &input) {
  String out;
  out.reserve(input.length());
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if ((uint8_t)c >= 0x20) {
      out += c;
    }
  }
  return out;
}

static String buildStatusJson() {
  String json = "{\"inputs\":[";
  for (uint8_t i = 0; i < PERIPH_INPUT_COUNT; i++) {
    if (i > 0) json += ",";
    json += Peripherals_ReadInput(i) ? "true" : "false";
  }
  json += "],\"relays\":[";
  for (uint8_t i = 0; i < PERIPH_RELAY_COUNT; i++) {
    if (i > 0) json += ",";
    json += Peripherals_GetRelay(i) ? "true" : "false";
  }
  json += "],\"analog\":" + String(Peripherals_GetAnalogOut());
  json += "}";
  return json;
}

static void sendResponse(Client &client, int code, const char *contentType, const String &body) {
  const char *statusText = (code == 200) ? "OK" : (code == 400) ? "Bad Request" : (code == 401) ? "Unauthorized" : "Not Found";

  client.print("HTTP/1.1 ");
  client.print(code);
  client.print(" ");
  client.println(statusText);
  client.print("Content-Type: ");
  client.println(contentType);
  client.print("Content-Length: ");
  client.println(body.length());
  client.println("Connection: close");
  client.println();
  client.print(body);
}

static void sendResponseP(Client &client, const char *contentType, const char *progmemBody) {
  size_t len = strlen_P(progmemBody);

  client.println("HTTP/1.1 200 OK");
  client.print("Content-Type: ");
  client.println(contentType);
  client.print("Content-Length: ");
  client.println(len);
  client.println("Connection: close");
  client.println();

  char buf[64];
  size_t sent = 0;
  while (sent < len) {
    size_t chunk = min(sizeof(buf), len - sent);
    memcpy_P(buf, progmemBody + sent, chunk);
    client.write((const uint8_t *)buf, chunk);
    sent += chunk;
  }
}

static void sendRedirect(Client &client, const String &location) {
  client.println("HTTP/1.1 302 Found");
  client.print("Location: ");
  client.println(location);
  client.println("Connection: close");
  client.println();
}

static void sendRedirectWithCookie(Client &client, const String &location, const String &cookie) {
  client.println("HTTP/1.1 302 Found");
  client.print("Location: ");
  client.println(location);
  client.print("Set-Cookie: ");
  client.println(cookie);
  client.println("Connection: close");
  client.println();
}

static void handleLoginPost(Client &client, const String &body) {
  String user = getFormField(body, "username");
  String pass = getFormField(body, "password");

  if (user == Settings_GetAuthUser() && pass == Settings_GetAuthPassword()) {
    sessionToken = String(esp_random(), HEX) + String(esp_random(), HEX);
    sendRedirectWithCookie(client, "/", String(SESSION_COOKIE_NAME) + "=" + sessionToken + "; Path=/");
  } else {
    sendResponseP(client, "text/html", LOGIN_HTML);
  }
}

static void handleRelayPost(Client &client, const String &body) {
  int indexPos = body.indexOf("\"index\"");
  int statePos = body.indexOf("\"state\"");
  if (indexPos < 0 || statePos < 0) {
    sendResponse(client, 400, "application/json", "{\"error\":\"bad json\"}");
    return;
  }

  uint8_t index = body.substring(body.indexOf(':', indexPos) + 1).toInt();
  String stateValue = body.substring(body.indexOf(':', statePos) + 1);
  stateValue.trim();
  bool state = stateValue.startsWith("true");

  Peripherals_SetRelay(index, state);
  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleAnalogPost(Client &client, const String &body) {
  int valuePos = body.indexOf("\"value\"");
  if (valuePos < 0) {
    sendResponse(client, 400, "application/json", "{\"error\":\"bad json\"}");
    return;
  }

  int value = body.substring(body.indexOf(':', valuePos) + 1).toInt();
  value = constrain(value, 0, 255);

  Peripherals_SetAnalogOut((uint8_t)value);
  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleSettingsPost(Client &client, const String &body) {
  String apSsid = getFormField(body, "ap_ssid");
  String apPassword = getFormField(body, "ap_password");
  String authUser = getFormField(body, "auth_user");
  String authPassword = getFormField(body, "auth_password");

  if (apSsid.length() == 0) apSsid = Settings_GetApSsid();
  if (apPassword.length() == 0) apPassword = Settings_GetApPassword();
  if (authUser.length() == 0) authUser = Settings_GetAuthUser();
  if (authPassword.length() == 0) authPassword = Settings_GetAuthPassword();

  Settings_SetApCredentials(apSsid, apPassword);
  Settings_SetAuthCredentials(authUser, authPassword);

  sendResponse(client, 200, "application/json", "{\"ok\":true}");
  delay(200);
  ESP.restart();
}

// Один запрос на всю страницу "Инфо" (версия + наработка + сеть разом) — раньше было 3
// отдельных fetch(), это 3 лишних одновременных соединения на плате, которая с несколькими
// параллельными соединениями справляется плохо.
static void handleInfoGet(Client &client) {
  String json = "{";
  json += "\"major\":" + String(fw_ver[0]);
  json += ",\"minor\":" + String(fw_ver[1]);
  json += ",\"patch\":" + String(fw_ver[2]);
  json += ",\"minutes\":" + String(Logic_GetRuntimeMinutes());
  json += ",\"fullCycles\":" + String(Logic_GetFullCycles());
  json += ",\"partialCycles\":" + String(Logic_GetPartialCycles());
  // Ethernet MAC фиксированный (ETH_MAC_ADDR), IP реальный только если линк поднят —
  // но отдаём как есть, страница сама решает, показывать ли по ethLinkUp/ethAvailable.
  json += ",\"ethAvailable\":" + String(Peripherals_EthAvailable() ? "true" : "false");
  json += ",\"ethLinkUp\":" + String(Peripherals_EthLinkUp() ? "true" : "false");
  json += ",\"ethMac\":\"" + Peripherals_EthGetMac() + "\"";
  json += ",\"ethIp\":\"" + Peripherals_EthGetIp().toString() + "\"";
  json += ",\"wifiConnected\":" + String(WifiClient_IsConnected() ? "true" : "false");
  json += ",\"wifiMac\":\"" + WiFi.macAddress() + "\""; // MAC станции — своя, отдельная от AP
  json += ",\"wifiIp\":\"" + WifiClient_GetIp().toString() + "\"";
  json += ",\"apMac\":\"" + WiFi.softAPmacAddress() + "\""; // MAC точки доступа — отличается от станции
  json += ",\"apIp\":\"" + WiFi.softAPIP().toString() + "\"";
  json += "}";
  sendResponse(client, 200, "application/json", json);
}

static void handleAlgorithmSettingsGet(Client &client) {
  // Читаем из живого logic_set (logic.cpp), а не из NVS — он всегда синхронен
  // с последним сохранённым значением, лишний раз лезть во флеш незачем.
  String json = "{\"di7On\":" + String(logic_set.di7.on ? "true" : "false") +
                ",\"presenceOffDelaySec\":" + String(logic_set.di7.delay_off) +
                ",\"di2Cycle\":" + String(logic_set.di2.cycle) +
                ",\"ch1Imp\":" + String(logic_set.ch1.imp) +
                ",\"ao1Max\":" + String(logic_set.ao1.max) +
                ",\"ao1F1Time\":" + String(logic_set.ao1.f1_time) +
                ",\"ao1F1Pwr\":" + String(logic_set.ao1.f1_pwr) +
                ",\"ao1F2Time\":" + String(logic_set.ao1.f2_time) +
                ",\"ao1F2Pwr\":" + String(logic_set.ao1.f2_pwr) +
                ",\"ao1F3Time\":" + String(logic_set.ao1.f3_time) +
                ",\"ao1F3Pwr\":" + String(logic_set.ao1.f3_pwr) +
                ",\"ao1DelayOff\":" + String(logic_set.ao1.delay_off) +
                ",\"ch6Inp\":" + String(logic_set.ch6.inp) +
                ",\"ch6Delay\":" + String(logic_set.ch6.delay) + "}";
  sendResponse(client, 200, "application/json", json);
}

// Обрезает целое из формы в [minValue, maxValue].
static int clampFormInt(const String &body, const char *field, int minValue, int maxValue) {
  int value = getFormField(body, field).toInt();
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  return value;
}

static void handleAlgorithmSettingsPost(Client &client, const String &body) {
  // Применяем сразу в работающей логике и сохраняем весь блок настроек в NVS разом —
  // без перезагрузки, в отличие от handleSettingsPost() (там AP/Wi-Fi требуют рестарта).
  logic_set.di7.on = getFormField(body, "di7_on") == "1";
  logic_set.di7.delay_off = (uint16_t)clampFormInt(body, "presence_off_delay_sec", 1, 300);
  logic_set.di2.cycle = (uint16_t)clampFormInt(body, "di2_cycle", 60, 300);
  logic_set.ch1.imp = (uint8_t)clampFormInt(body, "ch1_imp", 0, 30);
  logic_set.ao1.max = (uint8_t)clampFormInt(body, "ao1_max", 1, 100);
  logic_set.ao1.f1_time = (uint8_t)clampFormInt(body, "ao1_f1_time", 1, 30);
  logic_set.ao1.f1_pwr = (uint8_t)clampFormInt(body, "ao1_f1_pwr", 1, 100);
  logic_set.ao1.f2_time = (uint8_t)clampFormInt(body, "ao1_f2_time", 1, 30);
  logic_set.ao1.f2_pwr = (uint8_t)clampFormInt(body, "ao1_f2_pwr", 1, 100);
  logic_set.ao1.f3_time = (uint8_t)clampFormInt(body, "ao1_f3_time", 1, 30);
  logic_set.ao1.f3_pwr = (uint8_t)clampFormInt(body, "ao1_f3_pwr", 1, 100);
  logic_set.ao1.delay_off = (uint8_t)clampFormInt(body, "ao1_delay_off", 1, 30);
  logic_set.ch6.inp = (uint16_t)clampFormInt(body, "ch6_inp", 200, 2000);
  logic_set.ch6.delay = (uint16_t)clampFormInt(body, "ch6_delay", 200, 2000);
  Settings_SetAlgorithmSettings(logic_set);

  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleWifiScan(Client &client) {
  sendResponse(client, 200, "application/json", WifiClient_ScanJson());
}

static void handleWifiConnectPost(Client &client, const String &body) {
  String ssid = getFormField(body, "ssid");
  String password = getFormField(body, "password");

  if (ssid.length() == 0) {
    sendResponse(client, 400, "application/json", "{\"error\":\"no ssid\"}");
    return;
  }

  WifiClient_Connect(ssid, password);
  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleWifiStatus(Client &client) {
  String json = "{";
  if (WifiClient_IsConnected()) {
    json += "\"connected\":true,\"ssid\":\"" + jsonEscape(WifiClient_GetSsid()) + "\",\"ip\":\"" + WifiClient_GetIp().toString() + "\"";
  } else {
    json += "\"connected\":false";
  }
  json += "}";
  sendResponse(client, 200, "application/json", json);
}

static void handleWifiForgetPost(Client &client) {
  WifiClient_Forget();
  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleFirmwareUpload(Client &client, int contentLength) {
  if (contentLength <= 0) {
    sendResponse(client, 400, "application/json", "{\"error\":\"no content-length\"}");
    return;
  }

  if (!Update.begin(contentLength)) {
    sendResponse(client, 400, "application/json", "{\"error\":\"not enough space\"}");
    return;
  }

  uint8_t buf[512];
  int received = 0;
  unsigned long lastDataAt = millis();

  while (received < contentLength) {
    int avail = client.available();
    if (avail <= 0) {
      if (!client.connected() || millis() - lastDataAt > 15000) break;
      delay(1);
      continue;
    }
    int toRead = min((int)sizeof(buf), min(avail, contentLength - received));
    int n = client.read(buf, toRead);
    if (n <= 0) break;
    Update.write(buf, n);
    received += n;
    lastDataAt = millis();
  }

  if (received != contentLength || !Update.end(true)) {
    sendResponse(client, 400, "application/json", "{\"error\":\"upload failed\"}");
    return;
  }

  sendResponse(client, 200, "application/json", "{\"ok\":true}");
  delay(200);
  ESP.restart();
}

// Разбирает FIRMWARE_UPDATE_URL вида "http://host/путь" на хост и путь — без библиотек
// парсинга URL, просто по первому "/" после "http://".
static void parseUpdateUrl(String &host, String &path) {
  String url = FIRMWARE_UPDATE_URL;
  url.replace("http://", "");
  int slashIdx = url.indexOf('/');
  if (slashIdx < 0) {
    host = url;
    path = "/";
  } else {
    host = url.substring(0, slashIdx);
    path = url.substring(slashIdx);
  }
}

// Качает уже открытое соединение (GET-заголовки + тело) в Update.write(), тем же способом,
// что и приём файла из браузера в handleFirmwareUpload() выше. Возвращает true при успехе.
static bool downloadOverEthernetClient(EthernetClient &client, const String &host, const String &path) {
  client.print("GET " + path + " HTTP/1.1\r\n");
  client.print("Host: " + host + "\r\n");
  client.print("Connection: close\r\n\r\n");

  unsigned long waitStart = millis();
  while (client.connected() && !client.available()) {
    if (millis() - waitStart > 15000) {
      client.stop();
      return false;
    }
    delay(10);
  }

  // Читаем заголовки ответа строка за строкой до пустой строки, попутно вытаскивая
  // Content-Length — без него не знаем, сколько байт заливать во флеш.
  int contentLength = 0;
  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break; // пустая строка — заголовки кончились
    if (line.startsWith("Content-Length:")) {
      contentLength = line.substring(16).toInt();
    }
  }

  if (contentLength <= 0 || !Update.begin(contentLength)) {
    client.stop();
    return false;
  }

  uint8_t buf[512];
  int received = 0;
  unsigned long lastDataAt = millis();
  while (received < contentLength) {
    int avail = client.available();
    if (avail <= 0) {
      if (!client.connected() || millis() - lastDataAt > 15000) break;
      delay(1);
      continue;
    }
    int toRead = min((int)sizeof(buf), min(avail, contentLength - received));
    int n = client.read(buf, toRead);
    if (n <= 0) break;
    Update.write(buf, n);
    received += n;
    lastDataAt = millis();
    updateProgressPercent = (received * 100) / contentLength;
  }
  client.stop();

  return received == contentLength && Update.end(true);
}

// Пробует Ethernet и Wi-Fi ПРЯМО СЕЙЧАС, а не по тому, что было при загрузке платы — кабель
// могли перетыкнуть из роутера в ноутбук (или наоборот) уже после старта. Реальная проверка —
// попытка connect() к настоящему хосту обновления: получилось — значит, Ethernet прямо сейчас
// смотрит в интернет. HTTPUpdate из ESP-IDF с EthernetClient работать не может (ждёт
// NetworkClient её же Wi-Fi-стека), поэтому для Ethernet GET и заливку во флеш делаем
// вручную; для Wi-Fi (WiFiClient совместим с NetworkClient) пользуемся готовым httpUpdate.
static void firmwareUpdateTask(void *param) {
  String host, path;
  parseUpdateUrl(host, path);

  if (Peripherals_EthAvailable()) {
    EthernetClient ethClient;
    if (ethClient.connect(host.c_str(), 80)) {
      if (downloadOverEthernetClient(ethClient, host, path)) {
        updateProgressPercent = 100;
        delay(500);
        ESP.restart();
        return;
      }
      updateProgressPercent = -2; // подключились, но скачать/прошить не вышло — Wi-Fi тут не поможет
      vTaskDelete(nullptr);
      return;
    }
    // connect() не удался — Ethernet сейчас либо не воткнут, либо смотрит не в роутер
    // (например, в ноутбук). Переходим к Wi-Fi.
  }

  if (WifiClient_IsConnected()) {
    WiFiClient wifiClient;
    httpUpdate.onProgress([](int cur, int total) {
      if (total > 0) updateProgressPercent = (cur * 100) / total;
    });
    t_httpUpdate_return ret = httpUpdate.update(wifiClient, FIRMWARE_UPDATE_URL);
    if (ret == HTTP_UPDATE_OK) {
      updateProgressPercent = 100;
      delay(500);
      ESP.restart();
      return;
    }
  }

  updateProgressPercent = -2; // ни Ethernet, ни Wi-Fi не дали дотянуться до интернета
  vTaskDelete(nullptr);
}

static void handleUpdateFromServerPost(Client &client) {
  if (updateProgressPercent >= 0 && updateProgressPercent < 100) {
    sendResponse(client, 400, "application/json", "{\"error\":\"already running\"}");
    return;
  }

  // Если чипа Ethernet вообще нет и Wi-Fi не подключён — точно некуда идти, говорим сразу,
  // не запуская задачу вхолостую. Если чип Ethernet есть — не решаем заранее, воткнут ли он
  // в роутер: сама задача проверит это живым connect() к серверу обновления, и при неудаче
  // сама же перейдёт на Wi-Fi (см. firmwareUpdateTask).
  if (!Peripherals_EthAvailable() && !WifiClient_IsConnected()) {
    sendResponse(client, 400, "application/json", "{\"error\":\"no internet\"}");
    return;
  }
  updateProgressPercent = 0;
  xTaskCreate(firmwareUpdateTask, "fw_update", 8192, nullptr, 1, nullptr);
  sendResponse(client, 200, "application/json", "{\"ok\":true}");
}

static void handleUpdateProgress(Client &client) {
  sendResponse(client, 200, "application/json", "{\"percent\":" + String(updateProgressPercent) + "}");
}

static void handleRequest(Client &client, const String &method, const String &path, const String &cookie, const String &body) {
  if (method == "GET" && path == "/login") {
    sendResponseP(client, "text/html", LOGIN_HTML);
    return;
  }
  if (method == "POST" && path == "/login") {
    handleLoginPost(client, body);
    return;
  }
  if (method == "GET" && path == "/logout") {
    sessionToken = "";
    sendRedirectWithCookie(client, "/info", String(SESSION_COOKIE_NAME) + "=; Path=/; Max-Age=0");
    return;
  }
  if (method == "GET" && path == "/style.css") {
    sendResponseP(client, "text/css", STYLE_CSS);
    return;
  }
  if (method == "GET" && path == "/script.js") {
    sendResponseP(client, "application/javascript", SCRIPT_JS);
    return;
  }
  if (method == "GET" && path == "/") {
    sendRedirect(client, "/info");
    return;
  }
  if (method == "GET" && path == "/info") {
    // Единственная страница, доступная без авторизации (кроме /login) — какую шапку
    // показать, решает сервер по isAuthorized(), а не браузер по факту наличия куки:
    // после /logout или перезагрузки контроллера старая кука в браузере может остаться,
    // но sessionToken на сервере уже сброшен, так что isAuthorized() всё равно даст false.
    sendResponseP(client, "text/html", isAuthorized(cookie) ? INFO_HTML : INFO_PUBLIC_HTML);
    return;
  }
  if (method == "GET" && path == API_PATH_INFO) {
    // Тоже без авторизации — страница "Инфо" сама тянет отсюда данные, даже когда её
    // смотрят без логина.
    handleInfoGet(client);
    return;
  }

  if (!isAuthorized(cookie)) {
    if (method == "GET") {
      // Неавторизованный переход (включая проверку "captive portal" при
      // подключении к Wi-Fi) уводим на публичную страницу "Инфо", а не на
      // форму входа — вход доступен там кнопкой "Вход".
      sendRedirect(client, "/info");
    } else {
      sendResponse(client, 401, "application/json", "{\"error\":\"unauthorized\"}");
    }
    return;
  }

  if (method == "GET" && path == "/connection") {
    sendResponseP(client, "text/html", CONNECTION_HTML);
  } else if (method == "GET" && path == "/settings") {
    sendResponseP(client, "text/html", SETTINGS_HTML);
  } else if (method == "GET" && path == "/wifi") {
    sendResponseP(client, "text/html", WIFI_HTML);
  } else if (method == "GET" && path == "/update") {
    sendResponseP(client, "text/html", UPDATE_HTML);
  } else if (method == "GET" && path == "/access") {
    sendResponseP(client, "text/html", ACCESS_HTML);
  } else if (method == "GET" && path == "/io") {
    sendResponseP(client, "text/html", IO_HTML);
  } else if (method == "GET" && path == API_PATH_STATUS) {
    testModeActive = true;
    testModeLastPingMs = millis();
    sendResponse(client, 200, "application/json", buildStatusJson());
  } else if (method == "POST" && path == API_PATH_RELAY) {
    handleRelayPost(client, body);
  } else if (method == "POST" && path == API_PATH_ANALOG) {
    handleAnalogPost(client, body);
  } else if (method == "POST" && path == API_PATH_SETTINGS) {
    handleSettingsPost(client, body);
  } else if (method == "GET" && path == API_PATH_SETTINGS_ALGORITHM) {
    handleAlgorithmSettingsGet(client);
  } else if (method == "POST" && path == API_PATH_SETTINGS_ALGORITHM) {
    handleAlgorithmSettingsPost(client, body);
  } else if (method == "GET" && path == API_PATH_WIFI_SCAN) {
    handleWifiScan(client);
  } else if (method == "POST" && path == API_PATH_WIFI_CONNECT) {
    handleWifiConnectPost(client, body);
  } else if (method == "GET" && path == API_PATH_WIFI_STATUS) {
    handleWifiStatus(client);
  } else if (method == "POST" && path == API_PATH_WIFI_FORGET) {
    handleWifiForgetPost(client);
  } else if (method == "POST" && path == API_PATH_UPDATE_FROM_SERVER) {
    handleUpdateFromServerPost(client);
  } else if (method == "GET" && path == API_PATH_UPDATE_PROGRESS) {
    handleUpdateProgress(client);
  } else if (method == "GET") {
    // captive portal: любой незнакомый путь уводим на главную страницу
    sendRedirect(client, "/");
  } else {
    sendResponse(client, 404, "text/plain", "Not found");
  }
}

static void serviceClient(Client &client) {
  if (!client) return;

  unsigned long waitStart = millis();
  while (!client.available() && client.connected() && millis() - waitStart < 1000) {
    delay(1);
  }
  if (!client.available()) {
    client.stop();
    return;
  }

  String requestLine = client.readStringUntil('\n');
  requestLine.trim();

  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = requestLine.indexOf(' ', firstSpace + 1);
  String method = requestLine.substring(0, firstSpace);
  String path = requestLine.substring(firstSpace + 1, secondSpace);

  int contentLength = 0;
  String cookieHeader;
  String headerLine;
  do {
    headerLine = client.readStringUntil('\n');
    headerLine.trim();
    if (headerLine.startsWith("Content-Length:")) {
      contentLength = headerLine.substring(16).toInt();
    } else if (headerLine.startsWith("Cookie:")) {
      cookieHeader = headerLine.substring(7);
      cookieHeader.trim();
    }
  } while (headerLine.length() > 0);

  // Загрузку прошивки пишем потоково, не буферизуя весь файл в String —
  // ОЗУ ESP32 не хватит на бинарник в несколько сотен килобайт/мегабайт.
  if (method == "POST" && path == API_PATH_UPDATE) {
    if (!isAuthorized(cookieHeader)) {
      sendResponse(client, 401, "application/json", "{\"error\":\"unauthorized\"}");
    } else {
      handleFirmwareUpload(client, contentLength);
    }
    client.stop();
    return;
  }

  String body;
  if (contentLength > 0) {
    body.reserve(contentLength);
    unsigned long bodyStart = millis();
    while ((int)body.length() < contentLength && millis() - bodyStart < 1000) {
      if (client.available()) {
        body += (char)client.read();
      }
    }
  }

  handleRequest(client, method, path, cookieHeader, body);

  delay(1);
  client.stop();
}

void Web_Init() {
  if (Peripherals_EthAvailable()) {
    ethServer.begin();
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(Settings_GetApSsid().c_str(), Settings_GetApPassword().c_str());
  dnsServer.start(53, "*", WiFi.softAPIP());
  apServer.begin();

  WifiClient_Init();
}

void Web_OtaBegin() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();
}

void Web_Loop() {
  ArduinoOTA.handle();
  dnsServer.processNextRequest();

  if (Peripherals_EthAvailable()) {
    EthernetClient ethClient = ethServer.available();
    serviceClient(ethClient);
  }

  WiFiClient apClient = apServer.accept();
  serviceClient(apClient);
}
