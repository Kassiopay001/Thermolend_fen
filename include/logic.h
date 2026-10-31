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

// Наработка и счётчики циклов — только для чтения снаружи (пишет их исключительно
// сама logic.cpp по окончании цикла), поэтому геттеры, а не extern-переменные.
uint32_t Logic_GetRuntimeMinutes();
uint32_t Logic_GetNoHeatCycles();
uint32_t Logic_GetHeatCycles();
