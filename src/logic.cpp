#include "logic.h"

#include "peripherals.h"
#include "settings.h"

// DI1 Аварийная остановка
// DI2 Кнопка старт без обогрева
// DI3 Кнопка старт с обогревом
// DI4 Не используется
// DI5 Не используется
// DI6 Не используется
// DI7 Не используется
// DI8 Не используется
//
// CH1 Управление ароматизацией
// CH2 Подсветка кнопки старт без обогрева
// CH3 Подсветка кнопки старт с обогревом
// CH4 Световой контур А (контур человека)
// CH5 Световой контур Б (дефлекторы)
// CH6 Световой контур В (цветотерапия)
// CH7 Включение нагрева
// CH8 Включение вентилятора (пуск частотника, скорость задаёт AO1)
//
// AO1 Скорость вентилятора (аналоговый вход частотника)
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

// Три состояния системы:
// MODE_IDLE    — не активна, привлекает внимание: А горит, Б мигает сериями, В — импульсами,
//                подсветки кнопок погашены. Обе кнопки работают — в том числе пока вентилятор
//                ещё плавно останавливается после цикла.
// MODE_NO_HEAT — старт без обогрева (DI2): только вентилятор, весь свет погашен, горит CH2.
//                DI2 игнорируется, DI3 переводит в MODE_HEAT с отсчётом цикла заново.
// MODE_HEAT    — старт с обогревом (DI3): Б и В горят постоянно, аромат первые CH1.imp сек,
//                нагрев вместе с вентилятором, горит CH3. Кнопки игнорируются.
// По окончании цикла — сразу MODE_IDLE (свет уже по программе ожидания), а вентилятор
// параллельно плавно останавливается за DIx.ramp_down своего режима.
enum LogicMode { MODE_IDLE, MODE_NO_HEAT, MODE_HEAT };
static LogicMode mode = MODE_IDLE;
static uint32_t cycleElapsedMs = 0; // сколько идёт текущий цикл (MODE_NO_HEAT/MODE_HEAT)

static DebouncedButton noHeatBtn;        // DI2
static DebouncedButton heatBtn;          // DI3
static bool emergencyStopActive = false; // DI1 — для определения фронта (сработала/снята)

// Время между вызовами Logic_Update() — по нему тикают цикл, наработка и вентилятор. Именно
// накоплением, а не от millis() на старте цикла: пока открыта страница "Тест", логика стоит,
// и цикл вместе с ней тоже должен стоять, а не досчитываться задним числом.
static unsigned long lastUpdateMs = 0;

// Вентилятор: fanPct — текущая мощность, %, плавно идёт к fanTargetPct со скоростью
// fanRatePctPerSec. Считается каждый вызов (а не раз в секунду), чтобы AO1 менялся ровно.
// - Разгон: цель AO1.speed, темп AO1.speed / DIx.ramp_up в секунду — с нуля ровно за ramp_up.
//   Если вентилятор уже крутится (DI3 нажали на разгоне без обогрева, старт во время
//   остановки) — тем же темпом от текущей мощности, без сброса в 0.
// - Остановка: цель 0, темп "мощность на момент окончания / DIx.ramp_down" — ровно за ramp_down
//   от любой мощности, чтобы ТЭН продувался всё заданное время.
// CH8 (пуск частотника) включён, пока есть цель или вентилятор ещё не доехал до нуля.
static float fanPct = 0;
static uint8_t fanTargetPct = 0;
static float fanRatePctPerSec = 0;

// Программа мигания в ожидании: серия из count вспышек по imp с паузами gap между ними, затем
// pause, и заново. Свет В (CH6) — та же программа с count = 1 (gap тогда не участвует). Все
// времена — x0.1 сек, как в настройках. Каждый вход в ожидание начинает со вспышки.
struct LightProgram {
  bool on;
  uint8_t flashNo;      // номер текущей вспышки в серии, с 1
  unsigned long stepMs; // момент начала текущей вспышки/паузы
};
static LightProgram ch5Program; // свет Б
static LightProgram ch6Program; // свет В

static void LightProgram_Start(LightProgram &p) {
  p.on = true;
  p.flashNo = 1;
  p.stepMs = millis();
}

// count сравнивается через "<", а не "==", чтобы уменьшение числа вспышек в настройках прямо
// посреди серии не сломало счёт — просто начнётся пауза между сериями.
static void LightProgram_Service(LightProgram &p, uint16_t imp, uint16_t gap, uint8_t count, uint16_t pause) {
  uint16_t stepDs = p.on ? imp : (p.flashNo < count ? gap : pause);
  if (millis() - p.stepMs < stepDs * 100UL) return;
  p.stepMs = millis();
  if (p.on) {
    p.on = false;
  } else {
    p.on = true;
    p.flashNo = p.flashNo < count ? p.flashNo + 1 : 1;
  }
}

// Все 8 реле под логикой — собираем маску и пишем одним махом и только при изменении: каждая
// запись — обмен по I2C, а Logic_Update() крутится в loop() без задержек. -1 — "неизвестно, что
// сейчас на реле" (при старте и после страницы "Тест"), тогда пишем гарантированно.
static int16_t relayMaskWritten = -1;

static void WriteRelays(uint8_t mask) {
  if (mask == relayMaskWritten) return;
  Peripherals_SetAllRelays(mask);
  relayMaskWritten = mask;
}

// Наработка (время реальной работы цикла, не аптайм устройства, остановка вентилятора после
// цикла не в счёт) и счётчики циклов — статистика для страницы "Инфо". Копится в RAM, в NVS
// пишется только по окончании цикла (EndCycle) и при прерывании его аварией: длина цикла
// ограничена сверху (DIx.cycle, максимум 5 минут), чаще писать незачем. Цикл засчитывается,
// только если отработал до конца; цикл без обогрева, переключённый на обогрев, идёт в счётчик
// "с обогревом"; прерванный аварией не засчитывается никуда.
static uint32_t runtimeMinutesTotal = 0; // суммарная наработка, мин
static uint32_t runtimeMsInMinute = 0;   // 0-59999, мс текущей неполной минуты
static uint32_t noHeatCyclesTotal = 0;   // циклов без обогрева
static uint32_t heatCyclesTotal = 0;     // циклов с обогревом

uint32_t Logic_GetRuntimeMinutes() { return runtimeMinutesTotal; }
uint32_t Logic_GetNoHeatCycles() { return noHeatCyclesTotal; }
uint32_t Logic_GetHeatCycles() { return heatCyclesTotal; }

static void SaveRuntimeStats() {
  RuntimeStats stats = {runtimeMinutesTotal, noHeatCyclesTotal, heatCyclesTotal};
  Settings_SetRuntimeStats(stats);
}

static void EnterIdle() {
  mode = MODE_IDLE;
  LightProgram_Start(ch5Program);
  LightProgram_Start(ch6Program);
}

// Старт цикла — и из ожидания, и переключение без обогрева -> с обогревом (отсчёт заново,
// аромат тоже заново). Вентилятор подхватит разгон сам в Logic_Update() — см. fanPct.
static void StartCycle(LogicMode newMode) {
  mode = newMode;
  cycleElapsedMs = 0;
}

// Цикл отработал до конца (DIx.cycle истёк): засчитываем, пишем статистику, вентилятор — на
// плавную остановку за ramp_down своего режима, сами — сразу в ожидание.
static void EndCycle() {
  uint8_t rampDownSec;
  if (mode == MODE_HEAT) {
    heatCyclesTotal++;
    rampDownSec = logic_set.di3.ramp_down;
  } else {
    noHeatCyclesTotal++;
    rampDownSec = logic_set.di2.ramp_down;
  }
  SaveRuntimeStats();
  fanTargetPct = 0;
  fanRatePctPerSec = fanPct / (rampDownSec ? rampDownSec : 1);
  EnterIdle();
}

void Logic_Init() {
  logic_set = Settings_GetAlgorithmSettings();
  RuntimeStats stats = Settings_GetRuntimeStats();
  runtimeMinutesTotal = stats.minutes;
  noHeatCyclesTotal = stats.noHeatCycles;
  heatCyclesTotal = stats.heatCycles;
  lastUpdateMs = millis();
  EnterIdle();
}

void Logic_Update() {
  unsigned long now = millis();
  uint32_t dtMs = now - lastUpdateMs;
  lastUpdateMs = now;

  // licenseOk объявлен в peripherals.h — пока устройство не активировано, алгоритм стоит
  // целиком (иначе нажатия кнопок копили бы пустые циклы в статистике NVS), а сами выходы
  // заглушены в Peripherals_Set*. Проверка раньше testModeActive — тест тоже не в обход.
  if (!licenseOk) return;

  // testModeActive объявлен в peripherals.h — пока true, страница "Тест" (/io)
  // держит выходы под ручным управлением, алгоритм ниже их не трогает. После выхода из теста
  // реле переписываем заново — что там оставила страница, неизвестно.
  if (testModeActive) {
    relayMaskWritten = -1;
    return;
  }

  // Кнопки опрашиваем всегда, даже когда нажатие будет проигнорировано (авария, цикл с
  // обогревом) — иначе зажатая в это время кнопка сработала бы потом сама.
  bool noHeatPressed = Peripherals_ButtonHeld(noHeatBtn, Peripherals_ReadInput(1), PERIPH_BUTTON_DEBOUNCE_MS);
  bool heatPressed = Peripherals_ButtonHeld(heatBtn, Peripherals_ReadInput(2), PERIPH_BUTTON_DEBOUNCE_MS);

  // DI1 — цепь аварийной остановки, в норме замкнута (true); при размыкании (false) — авария.
  // Гасим всё резко, включая вентилятор (без плавной остановки) и нагрев. Прерванный цикл не
  // засчитывается, но наработку по нему сохраняем. При снятии аварии — в ожидание.
  if (!Peripherals_ReadInput(0)) {
    if (!emergencyStopActive) {
      if (mode != MODE_IDLE) SaveRuntimeStats();
      mode = MODE_IDLE;
      fanPct = 0;
      fanTargetPct = 0;
      emergencyStopActive = true;
    }
    WriteRelays(0);
    Peripherals_SetAnalogOut(0);
    return;
  }
  if (emergencyStopActive) {
    emergencyStopActive = false;
    EnterIdle();
  }

  // DI3 работает из ожидания и из цикла без обогрева (отсчёт заново), DI2 — только из ожидания.
  if (heatPressed && mode != MODE_HEAT) {
    StartCycle(MODE_HEAT);
  } else if (noHeatPressed && mode == MODE_IDLE) {
    StartCycle(MODE_NO_HEAT);
  }

  if (mode != MODE_IDLE) {
    cycleElapsedMs += dtMs;
    runtimeMsInMinute += dtMs;
    if (runtimeMsInMinute >= 60000UL) {
      runtimeMsInMinute -= 60000UL;
      runtimeMinutesTotal++;
    }
    uint16_t cycleSec = mode == MODE_HEAT ? logic_set.di3.cycle : logic_set.di2.cycle;
    if (cycleElapsedMs >= cycleSec * 1000UL) EndCycle();
  }

  // Вентилятор (см. fanPct выше). Во время цикла цель и темп берём из живых настроек каждый
  // вызов — поменяли AO1.speed на странице, вентилятор плавно поедет к новой скорости.
  if (mode != MODE_IDLE) {
    uint8_t rampUpSec = mode == MODE_HEAT ? logic_set.di3.ramp_up : logic_set.di2.ramp_up;
    fanTargetPct = logic_set.ao1.speed;
    fanRatePctPerSec = (float)logic_set.ao1.speed / (rampUpSec ? rampUpSec : 1);
  }
  float stepPct = fanRatePctPerSec * dtMs / 1000.0f;
  if (fanPct < fanTargetPct) {
    fanPct = fminf(fanPct + stepPct, fanTargetPct);
  } else if (fanPct > fanTargetPct) {
    fanPct = fmaxf(fanPct - stepPct, fanTargetPct);
  }
  uint8_t pwm = (uint8_t)(fanPct * 255.0f / 100.0f + 0.5f);
  if (pwm != Peripherals_GetAnalogOut()) Peripherals_SetAnalogOut(pwm);

  if (mode == MODE_IDLE) {
    LightProgram_Service(ch5Program, logic_set.ch5.imp, logic_set.ch5.gap, logic_set.ch5.count, logic_set.ch5.pause);
    LightProgram_Service(ch6Program, logic_set.ch6.imp, 0, 1, logic_set.ch6.pause);
  }

  bool idle = mode == MODE_IDLE;
  bool heat = mode == MODE_HEAT;
  uint8_t mask = 0;
  if (heat && cycleElapsedMs < logic_set.ch1.imp * 1000UL) mask |= 1 << 0; // CH1 аромат
  if (mode == MODE_NO_HEAT) mask |= 1 << 1;                                  // CH2
  if (heat) mask |= 1 << 2;                                                  // CH3
  if (idle) mask |= 1 << 3;                                                  // CH4 свет А
  if (heat || (idle && ch5Program.on)) mask |= 1 << 4;                       // CH5 свет Б
  if (heat || (idle && ch6Program.on)) mask |= 1 << 5;                       // CH6 свет В
  if (heat) mask |= 1 << 6;                                                  // CH7 нагрев
  if (fanTargetPct > 0 || fanPct > 0) mask |= 1 << 7;                        // CH8 вентилятор
  WriteRelays(mask);
}
