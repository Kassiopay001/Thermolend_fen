#pragma once

#include <Arduino.h>

void WifiClient_Init();

String WifiClient_ScanJson();
bool WifiClient_Connect(const String &ssid, const String &password);
void WifiClient_Forget();

bool WifiClient_IsConnected();
String WifiClient_GetSsid();
IPAddress WifiClient_GetIp();
