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
  return settings;
}

void Settings_SetAlgorithmSettings(const AlgorithmSettings &settings) {
  prefs.putBool("di7_on", settings.di7.on);
  prefs.putUShort("di7_delay_off", settings.di7.delay_off);
}
