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

AlgorithmSettings Settings_GetAlgorithmSettings() {
  AlgorithmSettings settings;
  settings.di7.on = prefs.getBool("di7_on", SETTINGS_DEFAULT_DI7_ON);
  settings.di7.delay_off = prefs.getUShort("di7_delay_off", SETTINGS_DEFAULT_DI7_DELAY_OFF_SEC);
  settings.di2.cycle = prefs.getUShort("di2_cycle", SETTINGS_DEFAULT_DI2_CYCLE_SEC);
  settings.ch1.imp = prefs.getUChar("ch1_imp", SETTINGS_DEFAULT_CH1_IMP_SEC);
  settings.ao1.max = prefs.getUChar("ao1_max", SETTINGS_DEFAULT_AO1_MAX_PERCENT);
  settings.ao1.f1_time = prefs.getUChar("ao1_f1_time", SETTINGS_DEFAULT_AO1_F1_TIME_SEC);
  settings.ao1.f1_pwr = prefs.getUChar("ao1_f1_pwr", SETTINGS_DEFAULT_AO1_F1_PWR_PERCENT);
  settings.ao1.f2_time = prefs.getUChar("ao1_f2_time", SETTINGS_DEFAULT_AO1_F2_TIME_SEC);
  settings.ao1.f2_pwr = prefs.getUChar("ao1_f2_pwr", SETTINGS_DEFAULT_AO1_F2_PWR_PERCENT);
  settings.ao1.f3_time = prefs.getUChar("ao1_f3_time", SETTINGS_DEFAULT_AO1_F3_TIME_SEC);
  settings.ao1.f3_pwr = prefs.getUChar("ao1_f3_pwr", SETTINGS_DEFAULT_AO1_F3_PWR_PERCENT);
  settings.ao1.delay_off = prefs.getUChar("ao1_delay_off", SETTINGS_DEFAULT_AO1_DELAY_OFF_SEC);
  settings.ch6.inp = prefs.getUShort("ch6_inp", SETTINGS_DEFAULT_CH6_INP_MS);
  settings.ch6.delay = prefs.getUShort("ch6_delay", SETTINGS_DEFAULT_CH6_DELAY_MS);
  return settings;
}

void Settings_SetAlgorithmSettings(const AlgorithmSettings &settings) {
  prefs.putBool("di7_on", settings.di7.on);
  prefs.putUShort("di7_delay_off", settings.di7.delay_off);
  prefs.putUShort("di2_cycle", settings.di2.cycle);
  prefs.putUChar("ch1_imp", settings.ch1.imp);
  prefs.putUChar("ao1_max", settings.ao1.max);
  prefs.putUChar("ao1_f1_time", settings.ao1.f1_time);
  prefs.putUChar("ao1_f1_pwr", settings.ao1.f1_pwr);
  prefs.putUChar("ao1_f2_time", settings.ao1.f2_time);
  prefs.putUChar("ao1_f2_pwr", settings.ao1.f2_pwr);
  prefs.putUChar("ao1_f3_time", settings.ao1.f3_time);
  prefs.putUChar("ao1_f3_pwr", settings.ao1.f3_pwr);
  prefs.putUChar("ao1_delay_off", settings.ao1.delay_off);
  prefs.putUShort("ch6_inp", settings.ch6.inp);
  prefs.putUShort("ch6_delay", settings.ch6.delay);
}

RuntimeStats Settings_GetRuntimeStats() {
  RuntimeStats stats;
  stats.minutes = prefs.getUInt("rt_minutes", 0);
  stats.fullCycles = prefs.getUInt("rt_full", 0);
  stats.partialCycles = prefs.getUInt("rt_partial", 0);
  return stats;
}

void Settings_SetRuntimeStats(const RuntimeStats &stats) {
  prefs.putUInt("rt_minutes", stats.minutes);
  prefs.putUInt("rt_full", stats.fullCycles);
  prefs.putUInt("rt_partial", stats.partialCycles);
}
