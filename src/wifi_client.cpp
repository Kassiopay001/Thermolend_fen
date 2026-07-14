#include "wifi_client.h"

#include <WiFi.h>

#include "settings.h"

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

void WifiClient_Init() {
  String ssid = Settings_GetStaSsid();
  if (ssid.length() == 0) return;

  String password = Settings_GetStaPassword();
  if (password.length() == 0) {
    WiFi.begin(ssid.c_str());
  } else {
    WiFi.begin(ssid.c_str(), password.c_str());
  }
}

String WifiClient_ScanJson() {
  int count = WiFi.scanNetworks();

  String json = "[";
  for (int i = 0; i < count; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true");
    json += "}";
  }
  json += "]";

  WiFi.scanDelete();
  return json;
}

struct WifiConnectParams {
  String ssid;
  String password;
};

static void wifiConnectTask(void *param) {
  WifiConnectParams *p = (WifiConnectParams *)param;
  String ssid = p->ssid;
  String password = p->password;
  delete p;

  if (password.length() == 0) {
    WiFi.begin(ssid.c_str());
  } else {
    WiFi.begin(ssid.c_str(), password.c_str());
  }

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(200);
  }

  // Логин/пароль сохраняем только если подключение реально удалось —
  // иначе в памяти будут залипать неверные данные.
  if (WiFi.status() == WL_CONNECTED) {
    Settings_SetStaCredentials(ssid, password);
  }

  vTaskDelete(nullptr);
}

bool WifiClient_Connect(const String &ssid, const String &password) {
  WifiConnectParams *params = new WifiConnectParams{ssid, password};
  xTaskCreate(wifiConnectTask, "wifi_connect", 4096, params, 1, nullptr);
  return true;
}

void WifiClient_Forget() {
  Settings_ClearStaCredentials();
  WiFi.disconnect(false, true);
}

bool WifiClient_IsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WifiClient_GetSsid() {
  return WiFi.SSID();
}

IPAddress WifiClient_GetIp() {
  return WiFi.localIP();
}
