#pragma once

#include <Arduino.h>

#define SETTINGS_DEFAULT_AP_SSID "ESP32-Relay-Controller"
#define SETTINGS_DEFAULT_AP_PASSWORD "12345678"
#define SETTINGS_DEFAULT_AUTH_USER "admin"
#define SETTINGS_DEFAULT_AUTH_PASSWORD "admin"
#define SETTINGS_DEFAULT_DI7_ON true
#define SETTINGS_DEFAULT_DI7_DELAY_OFF_SEC 5
#define SETTINGS_DEFAULT_DI2_CYCLE_SEC 180
#define SETTINGS_DEFAULT_CH1_IMP_SEC 5
#define SETTINGS_DEFAULT_AO1_MAX_PERCENT 100
#define SETTINGS_DEFAULT_AO1_F1_TIME_SEC 10
#define SETTINGS_DEFAULT_AO1_F1_PWR_PERCENT 30
#define SETTINGS_DEFAULT_AO1_F2_TIME_SEC 10
#define SETTINGS_DEFAULT_AO1_F2_PWR_PERCENT 60
#define SETTINGS_DEFAULT_AO1_F3_TIME_SEC 5
#define SETTINGS_DEFAULT_AO1_F3_PWR_PERCENT 100
#define SETTINGS_DEFAULT_AO1_DELAY_OFF_SEC 10
#define SETTINGS_DEFAULT_CH6_INP_MS 800
#define SETTINGS_DEFAULT_CH6_DELAY_MS 800

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

  struct {
    uint16_t cycle; // DI2.cycle — длина цикла, сек (60-300)
  } di2;

  struct {
    uint8_t imp; // CH1.imp — длительность импульса ароматизации, сек (0-30)
  } ch1;

  struct {
    uint8_t max;     // AO1.max — ограничение вентилятора, % (1-100)
    uint8_t f1_time; // AO1.f1_time — 1 фаза обдува, длительность, сек (1-30)
    uint8_t f1_pwr;  // AO1.f1_pwr — 1 фаза обдува, мощность, % (1-100)
    uint8_t f2_time; // AO1.f2_time — 2 фаза обдува, длительность, сек (1-30)
    uint8_t f2_pwr;  // AO1.f2_pwr — 2 фаза обдува, мощность, % (1-100)
    uint8_t f3_time;  // AO1.f3_time — 3 фаза обдува, длительность, сек (1-30)
    uint8_t f3_pwr;   // AO1.f3_pwr — 3 фаза обдува, мощность, % (1-100)
    uint8_t delay_off; // AO1.delay_off — плавная остановка обдува, сек (1-30)
  } ao1;

  struct {
    uint16_t inp;   // CH6.inp — длительность импульса управления подсветкой, мсек (200-2000)
    uint16_t delay; // CH6.delay — длительность паузы управления подсветкой, мсек (200-2000)
  } ch6;
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

// Наработка (время реальной работы цикла) и счётчики циклов — статистика, а не
// пользовательская настройка, но хранится тем же способом (NVS). Один Get/Set пары на
// всю тройку значений — logic.cpp пишет её раз за цикл (см. StopCycle), не чаще.
struct RuntimeStats {
  uint32_t minutes;       // суммарная наработка, мин
  uint32_t fullCycles;    // количество полностью отработанных циклов
  uint32_t partialCycles; // количество прерванных (не полностью отработанных) циклов
};

RuntimeStats Settings_GetRuntimeStats();
void Settings_SetRuntimeStats(const RuntimeStats &stats);
