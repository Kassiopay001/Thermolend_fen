#include "logic.h"

#include "peripherals.h"
#include "settings.h"

// DI1 Аварийная остановка
// DI2 Кнопка старт/стоп
// DI3 Кнопка вкл/выкл светомузыка
// DI4 Кнопка вкл/выкл хромотерапии
// DI5 Кнопка вкл/выкл нагрев
// DI6 Кнопка понижения потока воздуха
// DI7 Датчик присутствия
// DI8 Не используется
//
// CH1 Управление ароматизацией
// CH2 Подсветка кнопки старт/стоп
// CH3 Подсветка кнопки вкл/выкл светомузыка
// CH4 Подсветка кнопки вкл/выкл хромотерапии
// CH5 Подсветка кнопки вкл/выкл нагрев
// CH6 Управление светомузыкой
// CH7 Управление хромотерапией
// CH8 Управление нагревом
//
// AO1 Управление скоростью потока воздуха
//
// Peripherals_ReadInput(index);       // Прочитать состояние одного входа (index 0-7 = DI1-DI8)
// Peripherals_ReadAllInputs();        // Прочитать состояние всех входов одной битовой маской
// Peripherals_SetRelay(index, state); // Включить/выключить одно реле (index 0-7 = RO1-RO8)
// Peripherals_GetRelay(index);        // Прочитать состояние одного реле
// Peripherals_SetAllRelays(mask);     // Установить все реле разом битовой маской
// Peripherals_GetAllRelays();         // Прочитать состояние всех реле одной битовой маской
// Peripherals_SetAnalogOut(value);    // Установить аналоговый выход (ШИМ), value 0-255
// Peripherals_GetAnalogOut();         // Прочитать текущее значение аналогового выхода

// logic_set объявлена в logic.h — все настройки алгоритма разом (NVS), подробности
// в settings.h/AlgorithmSettings. Здесь только определение переменной.
AlgorithmSettings logic_set;

static DebouncedButton lightMusicBtn;    // DI3
static DebouncedButton chromotherapyBtn; // DI4
static DebouncedButton heatingBtn;       // DI5
static bool presenceWasActive = false;   // DI7
static bool presenceOffPending = false;  // ждём истечения задержки перед отключением
static unsigned long presenceLostMs = 0; // момент, когда DI7 разомкнулся

void Logic_Init() {
  logic_set = Settings_GetAlgorithmSettings();
}

void Logic_Update() {
  // testModeActive объявлен в peripherals.h — пока true, страница "Тест" (/io)
  // держит выходы под ручным управлением, алгоритм ниже их не трогает.
  if (testModeActive) return;

  // Быстрая проверка входов — каждый вызов Logic_Update(), без задержек.
  // DI1 — цепь аварийной остановки, в норме замкнута (true); при размыкании (false) — авария.
  if (!Peripherals_ReadInput(0)) {
    Peripherals_SetAllRelays(0x00);
    Peripherals_SetAnalogOut(0);
    // здесь же будет сброс цикла, когда опишем его состояние
    return;
  }

  // DI7 — датчик присутствия: logic_set.di7.on говорит, физически ли он установлен.
  // Если его нет — весь блок ниже просто не выполняется, CH3/CH4/CH5 полностью
  // отдаются кнопкам DI3/DI4/DI5 без вмешательства логики присутствия.
  if (logic_set.di7.on) {
    // При замыкании (человек пришёл) сразу включаем подсветку всех 3 кнопок. При
    // размыкании (человек ушёл) — не гасим сразу, а ждём logic_set.di7.delay_off
    // секунд (вдруг человек просто на миг вышел из зоны датчика); если за это время
    // он вернётся — отключение отменяется.
    bool presenceActive = Peripherals_ReadInput(6);
    if (presenceActive) {
      if (!presenceWasActive) { // человек пришёл (или успел вернуться до истечения задержки)
        Peripherals_SetRelay(2, true); // CH3
        Peripherals_SetRelay(3, true); // CH4
        Peripherals_SetRelay(4, true); // CH5
      }
      presenceOffPending = false; // отменяем отложенное отключение
    } else {
      if (presenceWasActive) { // человек только что пропал — запускаем отсчёт задержки
        presenceOffPending = true;
        presenceLostMs = millis();
      }
      if (presenceOffPending && millis() - presenceLostMs >= (unsigned long)logic_set.di7.delay_off * 1000UL) {
        Peripherals_SetRelay(2, false); // CH3
        Peripherals_SetRelay(3, false); // CH4
        Peripherals_SetRelay(4, false); // CH5
        presenceOffPending = false;
      }
    }
    presenceWasActive = presenceActive;
  }

  // DI3 — кнопка светомузыки: Peripherals_ButtonHeld() вернёт true один раз, когда кнопку
  // удержали дольше PERIPH_BUTTON_DEBOUNCE_MS (константа железа, не настройка) — тогда
  // переключаем CH3 (подсветку этой кнопки) в обратное состояние.
  if (Peripherals_ButtonHeld(lightMusicBtn, Peripherals_ReadInput(2), PERIPH_BUTTON_DEBOUNCE_MS)) {
    Peripherals_SetRelay(2, !Peripherals_GetRelay(2)); // CH3
  }

  // DI4 — кнопка хромотерапии: та же механика, переключаем её собственную подсветку CH4.
  if (Peripherals_ButtonHeld(chromotherapyBtn, Peripherals_ReadInput(3), PERIPH_BUTTON_DEBOUNCE_MS)) {
    Peripherals_SetRelay(3, !Peripherals_GetRelay(3)); // CH4
  }

  // DI5 — кнопка нагрева: та же механика, переключаем её собственную подсветку CH5.
  if (Peripherals_ButtonHeld(heatingBtn, Peripherals_ReadInput(4), PERIPH_BUTTON_DEBOUNCE_MS)) {
    Peripherals_SetRelay(4, !Peripherals_GetRelay(4)); // CH5
  }

  // Основная логика принятия решений — не чаще 1 раза в секунду.
  static unsigned long lastDecisionMs = 0;
  if (millis() - lastDecisionMs < 1000) return;
  lastDecisionMs = millis();

  // Твой алгоритм — начиная отсюда.
}
