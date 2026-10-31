#pragma once

#include <Arduino.h>

#define SETTINGS_DEFAULT_AP_SSID "ML Systems-Controller"
#define SETTINGS_DEFAULT_AP_PASSWORD "12345678"
#define SETTINGS_DEFAULT_AUTH_USER "admin"
#define SETTINGS_DEFAULT_AUTH_PASSWORD "admin"
#define SETTINGS_DEFAULT_DI2_CYCLE_SEC 180
#define SETTINGS_DEFAULT_DI2_RAMP_UP_SEC 8
#define SETTINGS_DEFAULT_DI2_RAMP_DOWN_SEC 10
#define SETTINGS_DEFAULT_DI3_CYCLE_SEC 180
#define SETTINGS_DEFAULT_DI3_RAMP_UP_SEC 15
#define SETTINGS_DEFAULT_DI3_RAMP_DOWN_SEC 30
#define SETTINGS_DEFAULT_CH1_IMP_SEC 5
#define SETTINGS_DEFAULT_CH5_IMP_DS 5     // 0.5 сек
#define SETTINGS_DEFAULT_CH5_GAP_DS 5     // 0.5 сек
#define SETTINGS_DEFAULT_CH5_COUNT 3
#define SETTINGS_DEFAULT_CH5_PAUSE_DS 50  // 5.0 сек
#define SETTINGS_DEFAULT_CH6_IMP_DS 30    // 3.0 сек
#define SETTINGS_DEFAULT_CH6_PAUSE_DS 200 // 20.0 сек
#define SETTINGS_DEFAULT_AO1_SPEED_PERCENT 100

// Все настройки алгоритма (logic.cpp) — один общий блок в NVS. Когда добавляется
// новый параметр, он просто дописывается сюда и в оба Settings_*AlgorithmSettings() —
// отдельных Get/Set-функций на каждый параметр больше не заводим, их будет много.
// Группируем по входу/выходу, к которому параметр относится (имя мне присылает
// пользователь в формате DI7.delay_off и т.п.) — отсюда вложенные структуры.
// Времена подсветки (CH5/CH6) — в десятых долях секунды (_DS), т.к. вспышки бывают
// по 0.5 сек; на странице настроек показываются и вводятся в секундах с шагом 0.1.
struct AlgorithmSettings {
  struct {
    uint16_t cycle;    // DI2.cycle — режим без обогрева, длина цикла, сек (60-300)
    uint8_t ramp_up;   // DI2.ramp_up — режим без обогрева, разгон вентилятора, сек (1-60)
    uint8_t ramp_down; // DI2.ramp_down — режим без обогрева, остановка вентилятора, сек (1-120)
  } di2;

  struct {
    uint16_t cycle;    // DI3.cycle — режим с обогревом, длина цикла, сек (60-300)
    uint8_t ramp_up;   // DI3.ramp_up — режим с обогревом, разгон вентилятора, сек (1-60)
    uint8_t ramp_down; // DI3.ramp_down — режим с обогревом, остановка (продувка ТЭНа), сек (1-120)
  } di3;

  struct {
    uint8_t imp; // CH1.imp — длительность импульса ароматизации, сек (0-30), 0 — не включать
  } ch1;

  struct {
    uint16_t imp;   // CH5.imp — свет Б, длительность вспышки, x0.1 сек (0.1-10.0)
    uint16_t gap;   // CH5.gap — свет Б, пауза между вспышками, x0.1 сек (0.1-10.0)
    uint8_t count;  // CH5.count — свет Б, вспышек в серии (1-10)
    uint16_t pause; // CH5.pause — свет Б, пауза между сериями, x0.1 сек (0.1-60.0)
  } ch5;

  struct {
    uint16_t imp;   // CH6.imp — свет В, длительность свечения, x0.1 сек (0.1-60.0)
    uint16_t pause; // CH6.pause — свет В, пауза, x0.1 сек (0.1-120.0)
  } ch6;

  struct {
    uint8_t speed; // AO1.speed — скорость вентилятора, общая для обоих режимов, % (1-100)
  } ao1;
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
// всю тройку значений — logic.cpp пишет её раз за цикл (см. EndCycle), не чаще.
struct RuntimeStats {
  uint32_t minutes;      // суммарная наработка, мин
  uint32_t noHeatCycles; // отработанных до конца циклов без обогрева
  uint32_t heatCycles;   // отработанных до конца циклов с обогревом
};

RuntimeStats Settings_GetRuntimeStats();
void Settings_SetRuntimeStats(const RuntimeStats &stats);
