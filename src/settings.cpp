#include "settings.h"

#include <Preferences.h>

static Preferences prefs;

void Settings_Init() {
  prefs.begin("settings", false);
}

String Settings_GetApSsid() {
  return prefs.getString("ap_ssid", SETTINGS_DEFAULT_AP_SSID);
}

String Settings_GetApPassword() {
  return prefs.getString("ap_pass", SETTINGS_DEFAULT_AP_PASSWORD);
}

void Settings_SetApCredentials(const String &ssid, const String &password) {
  prefs.putString("ap_ssid", ssid);
  prefs.putString("ap_pass", password);
}

String Settings_GetAuthUser() {
  return prefs.getString("auth_user", SETTINGS_DEFAULT_AUTH_USER);
}

String Settings_GetAuthPassword() {
  return prefs.getString("auth_pass", SETTINGS_DEFAULT_AUTH_PASSWORD);
}

void Settings_SetAuthCredentials(const String &user, const String &password) {
  prefs.putString("auth_user", user);
  prefs.putString("auth_pass", password);
}

String Settings_GetStaSsid() {
  return prefs.getString("sta_ssid", "");
}

String Settings_GetStaPassword() {
  return prefs.getString("sta_pass", "");
}

void Settings_SetStaCredentials(const String &ssid, const String &password) {
  prefs.putString("sta_ssid", ssid);
  prefs.putString("sta_pass", password);
}

void Settings_ClearStaCredentials() {
  prefs.remove("sta_ssid");
  prefs.remove("sta_pass");
}

// Ключи NVS от прошлых версий (до 2.0.0), которых больше нет в AlgorithmSettings/RuntimeStats —
// подчищаем при чтении настроек, чтобы не висели во флеше.
static const char *const OBSOLETE_KEYS[] = {
  "ch6_inp", "ch6_delay", "di7_on", "di7_delay_off", "ao1_max",
  "ao1_f1_time", "ao1_f1_pwr", "ao1_f2_time", "ao1_f2_pwr", "ao1_f3_time", "ao1_f3_pwr",
  "ao1_delay_off", "rt_full", "rt_partial",
};

AlgorithmSettings Settings_GetAlgorithmSettings() {
  AlgorithmSettings settings;
  settings.di2.cycle = prefs.getUShort("di2_cycle", SETTINGS_DEFAULT_DI2_CYCLE_SEC);
  settings.di2.ramp_up = prefs.getUChar("di2_ramp_up", SETTINGS_DEFAULT_DI2_RAMP_UP_SEC);
  settings.di2.ramp_down = prefs.getUChar("di2_ramp_down", SETTINGS_DEFAULT_DI2_RAMP_DOWN_SEC);
  settings.di3.cycle = prefs.getUShort("di3_cycle", SETTINGS_DEFAULT_DI3_CYCLE_SEC);
  settings.di3.ramp_up = prefs.getUChar("di3_ramp_up", SETTINGS_DEFAULT_DI3_RAMP_UP_SEC);
  settings.di3.ramp_down = prefs.getUChar("di3_ramp_down", SETTINGS_DEFAULT_DI3_RAMP_DOWN_SEC);
  settings.ch1.imp = prefs.getUChar("ch1_imp", SETTINGS_DEFAULT_CH1_IMP_SEC);
  settings.ch5.imp = prefs.getUShort("ch5_imp", SETTINGS_DEFAULT_CH5_IMP_DS);
  settings.ch5.gap = prefs.getUShort("ch5_gap", SETTINGS_DEFAULT_CH5_GAP_DS);
  settings.ch5.count = prefs.getUChar("ch5_count", SETTINGS_DEFAULT_CH5_COUNT);
  settings.ch5.pause = prefs.getUShort("ch5_pause", SETTINGS_DEFAULT_CH5_PAUSE_DS);
  settings.ch6.imp = prefs.getUShort("ch6_imp", SETTINGS_DEFAULT_CH6_IMP_DS);
  settings.ch6.pause = prefs.getUShort("ch6_pause", SETTINGS_DEFAULT_CH6_PAUSE_DS);
  settings.ao1.speed = prefs.getUChar("ao1_speed", SETTINGS_DEFAULT_AO1_SPEED_PERCENT);
  for (const char *key : OBSOLETE_KEYS) {
    if (prefs.isKey(key)) prefs.remove(key);
  }
  return settings;
}

void Settings_SetAlgorithmSettings(const AlgorithmSettings &settings) {
  prefs.putUShort("di2_cycle", settings.di2.cycle);
  prefs.putUChar("di2_ramp_up", settings.di2.ramp_up);
  prefs.putUChar("di2_ramp_down", settings.di2.ramp_down);
  prefs.putUShort("di3_cycle", settings.di3.cycle);
  prefs.putUChar("di3_ramp_up", settings.di3.ramp_up);
  prefs.putUChar("di3_ramp_down", settings.di3.ramp_down);
  prefs.putUChar("ch1_imp", settings.ch1.imp);
  prefs.putUShort("ch5_imp", settings.ch5.imp);
  prefs.putUShort("ch5_gap", settings.ch5.gap);
  prefs.putUChar("ch5_count", settings.ch5.count);
  prefs.putUShort("ch5_pause", settings.ch5.pause);
  prefs.putUShort("ch6_imp", settings.ch6.imp);
  prefs.putUShort("ch6_pause", settings.ch6.pause);
  prefs.putUChar("ao1_speed", settings.ao1.speed);
}

RuntimeStats Settings_GetRuntimeStats() {
  RuntimeStats stats;
  stats.minutes = prefs.getUInt("rt_minutes", 0);
  stats.noHeatCycles = prefs.getUInt("rt_noheat", 0);
  stats.heatCycles = prefs.getUInt("rt_heat", 0);
  return stats;
}

void Settings_SetRuntimeStats(const RuntimeStats &stats) {
  prefs.putUInt("rt_minutes", stats.minutes);
  prefs.putUInt("rt_noheat", stats.noHeatCycles);
  prefs.putUInt("rt_heat", stats.heatCycles);
}
