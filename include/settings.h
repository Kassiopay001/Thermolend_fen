#pragma once

#include <Arduino.h>

#define SETTINGS_DEFAULT_AP_SSID "ESP32-Relay-Controller"
#define SETTINGS_DEFAULT_AP_PASSWORD "12345678"
#define SETTINGS_DEFAULT_AUTH_USER "admin"
#define SETTINGS_DEFAULT_AUTH_PASSWORD "admin"
#define SETTINGS_DEFAULT_DI7_ON true
#define SETTINGS_DEFAULT_DI7_DELAY_OFF_SEC 5

// Все настройки алгоритма (logic.cpp) — один общий блок в NVS. Когда добавляется
// новый параметр, он просто дописывается сюда и в оба Settings_*AlgorithmSettings() —
// отдельных Get/Set-функций на каждый параметр больше не заводим, их будет много.
// Группируем по входу/выходу, к которому параметр относится (имя мне присылает
// пользователь в формате DI7.delay_off и т.п.) — отсюда вложенные структуры.
struct AlgorithmSettings {
  struct {
    bool on;             // DI7.on — датчик присутствия физически установлен и используется
    uint16_t delay_off;  // DI7.delay_off — задержка отключения CH3/CH4/CH5, сек
  } di7;
};

void Settings_Init();

String Settings_GetApSsid();
String Settings_GetApPassword();
void Settings_SetApCredentials(const String &ssid, const String &password);

String Settings_GetAuthUser();
String Settings_GetAuthPassword();
void Settings_SetAuthCredentials(const String &user, const String &password);

String Settings_GetStaSsid();
String Settings_GetStaPassword();
void Settings_SetStaCredentials(const String &ssid, const String &password);
void Settings_ClearStaCredentials();

AlgorithmSettings Settings_GetAlgorithmSettings();
void Settings_SetAlgorithmSettings(const AlgorithmSettings &settings);
