#pragma once

#include <Arduino.h>

#include "settings.h"

// Текущие применённые настройки алгоритма — читаются напрямую (Logic_Update()) и
// пишутся напрямую (web.cpp при сохранении формы), без отдельных функций на каждое
// поле. Загружаются из NVS в Logic_Init(); чтобы применить и сохранить изменение —
// поменять поле здесь и вызвать Settings_SetAlgorithmSettings(logic_set).
extern AlgorithmSettings logic_set;

void Logic_Init();
void Logic_Update();
