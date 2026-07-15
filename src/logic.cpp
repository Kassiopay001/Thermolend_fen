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

static DebouncedButton startStopBtn;     // DI2
static DebouncedButton lightMusicBtn;    // DI3
static DebouncedButton chromotherapyBtn; // DI4
static DebouncedButton heatingBtn;       // DI5
static DebouncedButton airflowDownBtn;   // DI6
static bool presenceWasActive = false;   // DI7 — для определения фронта (пришёл/ушёл)
static bool emergencyStopActive = false; // DI1 — для определения фронта (сработала/снята)

// DI6 — понижение оборотов вентилятора, надстройка НАД AO1.max. Каждое нажатие переключает
// по кругу: 100% -> 75% -> 50% -> 25% -> 100%... Нигде не сохраняется (не настройка, а разовая
// ручная подстройка на время текущего цикла) и сбрасывается на 100% при каждом старте (DI2).
static const uint8_t DI6_LEVELS[4] = {100, 75, 50, 25};
static uint8_t di6LevelIndex = 0;

// Состояние цикла (между Старт и Стоп) — вся обработка ниже сделана простыми счётчиками
// внутри самого Logic_Update(), без отдельных функций: cycleActive держит, пока CH6/7/8
// живьём следуют за CH3/4/5; cycleRemainingSec — отсчёт длины самого цикла (DI2.cycle);
// aromaRemainingSec — сколько ещё секунд гореть CH1 (импульс ароматизации); fanPhase/
// fanPhaseElapsedSec/... — разгон AO1 по трём фазам; fanRampDownActive/... — плавное
// гашение AO1 после остановки цикла. Всё это тикает раз в секунду в блоке "Основная логика
// принятия решений" ниже — секундная точность тут более чем достаточна, а отдельные функции
// ради этого не нужны.
static bool cycleActive = false;
static uint16_t cycleRemainingSec = 0;  // отсчёт длины цикла, секунд (DI2.cycle)
static uint8_t aromaRemainingSec = 0;   // 0 = импульс CH1 сейчас не идёт
static uint8_t fanPhase = 0;            // 0 = разгон не идёт/завершён, 1/2/3 = текущая фаза
static uint8_t fanPhaseElapsedSec = 0;  // сколько секунд уже прошло в текущей фазе
static uint8_t fanPhaseStartPct = 0;    // % мощности в начале текущей фазы
static uint8_t fanPhaseTargetPct = 0;   // % мощности — цель текущей фазы
static uint8_t fanPhaseDurationSec = 0; // длительность текущей фазы, сек

static bool fanRampDownActive = false;      // идёт плавное гашение AO1 после остановки цикла
static uint8_t fanRampDownStartPwm = 0;     // значение ШИМ (0-255) на момент остановки
static uint8_t fanRampDownElapsedSec = 0;   // сколько секунд уже прошло в гашении
static uint8_t fanRampDownDurationSec = 0;  // = logic_set.ao1.delay_off на момент запуска

// CH6 (управление светомузыкой) — не просто реле вкл/выкл, а импульсный сигнал для внешнего
// контроллера светомузыки: количество импульсов выбирает программу на нём (1-5). Импульс —
// logic_set.ch6.inp мс, пауза между импульсами — logic_set.ch6.delay мс (обе в пределах
// 200-2000 мс, поэтому нужна мс-точность — ServiceCh6Pulses() тикает в быстром тике
// Logic_Update(), а не в decision-блоке раз в секунду).
static uint8_t ch6PulsesRemaining = 0;      // сколько ещё импульсов ОСТАЛОСЬ дать (0 = не идёт)
static bool ch6PulseOn = false;             // true = сейчас фаза импульса, false = пауза
static unsigned long ch6PulsePhaseAtMs = 0; // момент начала текущей фазы
static uint8_t ch6NextProgram = 2;          // следующая программа при включении света: 2..5 по кругу

static void StartCh6Pulses(uint8_t count) {
  Peripherals_SetRelay(5, true); // CH6 — начинаем сразу с импульса
  ch6PulsesRemaining = count;
  ch6PulseOn = true;
  ch6PulsePhaseAtMs = millis();
}

// Включение светомузыки (CH3 загорелась) — очередная программа 2..5 по кругу. Общая для
// момента старта цикла (если CH3 уже горит) и для нажатия DI3 прямо во время цикла.
static void PulseCh6NextOnProgram() {
  StartCh6Pulses(ch6NextProgram);
  ch6NextProgram = (ch6NextProgram >= 5) ? 2 : ch6NextProgram + 1;
}

static void ServiceCh6Pulses() {
  if (ch6PulsesRemaining == 0) return;
  unsigned long phaseDurationMs = ch6PulseOn ? logic_set.ch6.inp : logic_set.ch6.delay;
  if (millis() - ch6PulsePhaseAtMs < phaseDurationMs) return;
  ch6PulsePhaseAtMs = millis();
  if (ch6PulseOn) {
    Peripherals_SetRelay(5, false); // CH6 — импульс закончился
    ch6PulsesRemaining--;
    if (ch6PulsesRemaining > 0) ch6PulseOn = false; // ещё будут импульсы — уходим в паузу
  } else {
    Peripherals_SetRelay(5, true); // CH6 — пауза закончилась, следующий импульс
    ch6PulseOn = true;
  }
}

// Наработка (время реальной работы цикла, не аптайм устройства) и счётчики циклов —
// статистика для страницы "Инфо", копится в RAM и пишется в NVS только в момент остановки
// цикла (см. StopCycle). Чаще писать незачем: длина цикла ограничена сверху logic_set.di2.cycle
// (максимум 300 сек = 5 минут), так что накопить есть чему только в пределах одного цикла, а
// цикл в любом случае заканчивается через StopCycle(). runtimeMinutesTotal — источник истины
// для отображения, обновляется сразу по факту (не только после записи в NVS).
static uint32_t runtimeMinutesTotal = 0;    // суммарная наработка, мин
static uint16_t runtimeSecondsInMinute = 0; // 0-59, секунды текущей неполной минуты
static uint32_t fullCyclesTotal = 0;        // циклы, отработавшие полностью (до конца DI2.cycle)
static uint32_t partialCyclesTotal = 0;     // циклы, прерванные раньше (Стоп/уход человека)

uint32_t Logic_GetRuntimeMinutes() { return runtimeMinutesTotal; }
uint32_t Logic_GetFullCycles() { return fullCyclesTotal; }
uint32_t Logic_GetPartialCycles() { return partialCyclesTotal; }

// Полная остановка цикла — гасит вообще всё, что цикл включал: обе группы подсветок
// (CH2 и CH3/4/5) и CH7/CH8 — сразу. CH6 не гасится напрямую, а получает программу 1 (см.
// StartCh6Pulses выше) — один импульс как сигнал внешнему контроллеру светомузыки "цикл
// закончен". Ароматизация (CH1) гасится сразу; вентилятор (AO1) — не резко, а плавно от
// текущего значения до 0 за logic_set.ao1.delay_off секунд (см. fanRampDown* и decision-блок
// ниже). Используется в трёх местах: явное нажатие Стоп без датчика присутствия, автоматическое
// истечение задержки присутствия (человек ушёл и не вернулся) и истечение времени самого
// цикла (DI2.cycle) — для всех это одно и то же событие "цикл закончен", поэтому не
// дублируем логику остановки. completedFully — цикл дошёл до конца сам (таймер DI2.cycle) или
// был прерван раньше (Стоп/уход человека) — от этого зависит, в какой из двух счётчиков
// циклов он попадёт.
static void CancelLightsOffDelay(); // определена ниже — нужна здесь, чтобы снять зависшую
                                     // задержку, если цикл уже закончился другим путём раньше
static void StopCycle(bool completedFully) {
  cycleActive = false;
  cycleRemainingSec = 0;
  CancelLightsOffDelay(); // на случай, если задержка присутствия ещё висит с этого же цикла
  Peripherals_SetRelay(0, false); // CH1
  Peripherals_SetRelay(1, false); // CH2
  Peripherals_SetRelay(2, false); // CH3
  Peripherals_SetRelay(3, false); // CH4
  Peripherals_SetRelay(4, false); // CH5
  Peripherals_SetRelay(6, false); // CH7
  Peripherals_SetRelay(7, false); // CH8
  StartCh6Pulses(1); // CH6 — программа 1
  aromaRemainingSec = 0;
  fanPhase = 0; // прекращаем разгон, если он ещё шёл

  fanRampDownStartPwm = Peripherals_GetAnalogOut();
  fanRampDownElapsedSec = 0;
  fanRampDownDurationSec = logic_set.ao1.delay_off;
  fanRampDownActive = fanRampDownStartPwm > 0;
  if (!fanRampDownActive) Peripherals_SetAnalogOut(0); // уже 0 — гасить нечего

  if (completedFully) fullCyclesTotal++; else partialCyclesTotal++;
  RuntimeStats stats = {runtimeMinutesTotal, fullCyclesTotal, partialCyclesTotal};
  Settings_SetRuntimeStats(stats);
}

// Общий отложенный выключатель — запускается и потерей присутствия (DI7), и нажатием Стоп
// (DI2) при включённом датчике присутствия, чтобы не дублировать одну и ту же задержку/
// отмену в двух местах. НО: DI7 вооружает его при ЛЮБОЙ потере присутствия, даже когда
// никакой цикл не идёт (предцикловая подсветка "человек подошёл/отошёл", либо цикл уже
// закончился сам, а человек просто ещё не встал) — в этом случае по истечении задержки
// нужно просто погасить CH3/4/5, а НЕ считать это остановкой цикла (иначе один и тот же
// визит задвоится в статистике и лишний раз запишется в NVS). Поэтому at arm-time запоминаем
// forCycleStop — было ли это вооружено ради реально идущего цикла.
static bool lightsOffPending = false;      // идёт отсчёт
static bool lightsOffIsCycleStop = false;  // true — по истечении звать StopCycle(), false — просто погасить подсветку
static unsigned long lightsOffArmedMs = 0; // момент, когда отсчёт был запущен

static void ArmLightsOffDelay(bool forCycleStop) {
  if (!lightsOffPending) { // не перезапускаем отсчёт заново, если он уже идёт
    lightsOffPending = true;
    lightsOffArmedMs = millis();
    lightsOffIsCycleStop = forCycleStop;
  }
}

static void CancelLightsOffDelay() {
  lightsOffPending = false;
}

static void ServiceLightsOffDelay() {
  if (lightsOffPending && millis() - lightsOffArmedMs >= (unsigned long)logic_set.di7.delay_off * 1000UL) {
    lightsOffPending = false;
    if (lightsOffIsCycleStop) {
      StopCycle(false); // прервано — задержка присутствия истекла, человек не вернулся
    } else {
      // Цикл не идёт (предцикловая подсветка либо уже отработавший цикл) — гасим только
      // подсветку, статистику и остальное оборудование не трогаем.
      Peripherals_SetRelay(2, false); // CH3
      Peripherals_SetRelay(3, false); // CH4
      Peripherals_SetRelay(4, false); // CH5
    }
  }
}

void Logic_Init() {
  logic_set = Settings_GetAlgorithmSettings();
  RuntimeStats stats = Settings_GetRuntimeStats();
  runtimeMinutesTotal = stats.minutes;
  fullCyclesTotal = stats.fullCycles;
  partialCyclesTotal = stats.partialCycles;
}

// Считает и применяет реальный ШИМ AO1 из "базового" процента мощности (либо текущей точки
// разгона, либо уже устоявшегося значения после его окончания) с учётом AO1.max и ручной
// подстройки DI6. Общая для разгона и для устоявшегося состояния ПОСЛЕ него — иначе DI6,
// нажатый уже после того, как разгон закончился, было бы не к чему применить (ровно тот баг,
// который сейчас чиним).
static void ApplyFanOutputPercent(uint8_t basePct) {
  float_t limitedPct = basePct * (logic_set.ao1.max / 100.0f);
  limitedPct = limitedPct * (DI6_LEVELS[di6LevelIndex] / 100.0f);
  Peripherals_SetAnalogOut((uint8_t)(limitedPct * 255UL / 100UL));
}

void Logic_Update() {
  // testModeActive объявлен в peripherals.h — пока true, страница "Тест" (/io)
  // держит выходы под ручным управлением, алгоритм ниже их не трогает.
  if (testModeActive) return;

  // Быстрая проверка входов — каждый вызов Logic_Update(), без задержек.
  // DI1 — цепь аварийной остановки, в норме замкнута (true); при размыкании (false) — авария.
  if (!Peripherals_ReadInput(0)) {
    if (!emergencyStopActive) {
      // Авария только что сработала — полностью сбрасываем цикл (как будто он прервался):
      // подсветки, CH7/8, ароматизация, счётчики (незавершённый цикл), запись в NVS. CH6 тут
      // не гасим напрямую — StopCycle() сама включает её "программу 1" (короткий импульс-
      // сигнал внешнему контроллеру), это разрешено оставить и в аварии.
      StopCycle(false);
      // В обычной остановке AO1 гасится ПЛАВНО (fanRampDown) — тут нужно РЕЗКО, отменяем.
      fanRampDownActive = false;
      Peripherals_SetAnalogOut(0);
      emergencyStopActive = true;
    }
    // Пока авария активна: жёстко держим все реле (кроме CH6 — им управляет импульсная
    // программа выше, даём ей доиграть) и AO1 в нуле, чтобы ничего не включилось раньше
    // времени. При снятии аварии cycleActive уже false — само ничего не запустится, нужен
    // новый ручной Старт.
    Peripherals_SetRelay(0, false); // CH1
    Peripherals_SetRelay(1, false); // CH2
    Peripherals_SetRelay(2, false); // CH3
    Peripherals_SetRelay(3, false); // CH4
    Peripherals_SetRelay(4, false); // CH5
    Peripherals_SetRelay(6, false); // CH7
    Peripherals_SetRelay(7, false); // CH8
    Peripherals_SetAnalogOut(0);
    ServiceCh6Pulses(); // чтобы импульс "программы 1" на CH6 сам доиграл и погас
    return;
  }
  emergencyStopActive = false;

  // DI7 — датчик присутствия: logic_set.di7.on говорит, физически ли он установлен.
  // Если его нет — весь блок ниже просто не выполняется, CH3/CH4/CH5 полностью
  // отдаются кнопкам DI3/DI4/DI5 без вмешательства логики присутствия.
  if (logic_set.di7.on) {
    // При замыкании (человек пришёл) сразу включаем подсветку всех 3 кнопок и отменяем
    // отложенное выключение, если оно шло (например, было запущено кнопкой Стоп, а человек
    // никуда не уходил или успел вернуться). При размыкании (человек ушёл) — не гасим сразу,
    // а запускаем общий отложенный выключатель (см. ArmLightsOffDelay выше), который через
    // logic_set.di7.delay_off секунд выключит CH3/CH4/CH5, если человек не вернётся.
    bool presenceActive = Peripherals_ReadInput(6);
    if (presenceActive) {
      if (!presenceWasActive) { // человек только что пришёл (или вернулся)
        Peripherals_SetRelay(2, true); // CH3
        Peripherals_SetRelay(3, true); // CH4
        Peripherals_SetRelay(4, true); // CH5
        CancelLightsOffDelay();
      }
    } else if (presenceWasActive) { // человек только что пропал — запускаем отсчёт задержки
      // forCycleStop = cycleActive: если цикл сейчас реально идёт, по истечении задержки это
      // его прерывание (StopCycle). Если цикла нет (предцикловая подсветка либо цикл уже
      // закончился сам, а человек просто ещё не встал) — по истечении просто гасим подсветку,
      // не считаем это отдельным (лишним) циклом и не пишем в NVS.
      ArmLightsOffDelay(cycleActive);
    }
    presenceWasActive = presenceActive;
  }

  // DI2 — кнопка старт/стоп: та же механика антидребезга, что и у DI3/DI4/DI5, но
  // включаем/выключаем ей CH2 (собственную подсветку). Особые случаи с CH3/CH4/CH5:
  // - без датчика присутствия (logic_set.di7.on == false) кнопка ими управляет полностью
  //   сама — включает сразу при старте, гасит сразу при стопе;
  // - с датчиком присутствия ими управляет он, кнопка тут ничего не включает и не гасит
  //   напрямую, а лишь запускает тот же отложенный выключатель, что и уход человека (Стоп),
  //   либо отменяет его, если он шёл (Старт передумали нажать).
  if (Peripherals_ButtonHeld(startStopBtn, Peripherals_ReadInput(1), PERIPH_BUTTON_DEBOUNCE_MS)) {
    bool turningOn = !Peripherals_GetRelay(1); // CH2 сейчас выключен — значит, включаем цикл
    Peripherals_SetRelay(1, !Peripherals_GetRelay(1)); // CH2
    if (turningOn) {
      if (!logic_set.di7.on) {
        Peripherals_SetRelay(2, true); // CH3
        Peripherals_SetRelay(3, true); // CH4
        Peripherals_SetRelay(4, true); // CH5
      }
      CancelLightsOffDelay();
      cycleActive = true;
      cycleRemainingSec = logic_set.di2.cycle; // отсчёт длины цикла, досчитывает decision-блок ниже
      fanRampDownActive = false; // на случай, если предыдущее гашение ещё не успело завершиться
      di6LevelIndex = 0; // DI6 — каждый новый цикл стартует на 100% оборотов

      // CH6 — если светомузыка (CH3) уже горит на момент старта, запускаем на ней очередную
      // программу 2..5. Если не горит — ничего не шлём, остаёмся на программе 1 (см. также
      // DI3 ниже — та же логика включения/выключения работает и прямо во время цикла).
      if (Peripherals_GetRelay(2)) PulseCh6NextOnProgram(); // CH3

      // Ароматизация — включаем CH1 сразу, досчитывает секунды decision-блок ниже.
      aromaRemainingSec = logic_set.ch1.imp;
      Peripherals_SetRelay(0, aromaRemainingSec > 0); // CH1

      // Разгон вентилятора — запускаем 1-ю фазу, дальше её продвигает decision-блок ниже.
      fanPhase = 1;
      fanPhaseElapsedSec = 0;
      fanPhaseStartPct = 0;
      fanPhaseTargetPct = logic_set.ao1.f1_pwr;
      fanPhaseDurationSec = logic_set.ao1.f1_time;
    } else if (!logic_set.di7.on) {
      // Без датчика присутствия некому потом досчитать задержку — останавливаем цикл сразу
      // и полностью (см. StopCycle выше): подсветки, CH6/7/8, ароматизация, вентилятор.
      StopCycle(false); // прервано вручную кнопкой Стоп
    } else {
      // С датчиком — сразу гасим только CH7/CH8, остальное (подсветки CH3/4/5, CH6-программа
      // 1, ароматизация, вентилятор) остановит ServiceLightsOffDelay() ниже, когда истечёт
      // задержка присутствия, если человек не вернётся раньше.
      cycleActive = false;
      Peripherals_SetRelay(6, false); // CH7
      Peripherals_SetRelay(7, false); // CH8
      ArmLightsOffDelay(true); // это реальная остановка цикла, а не предцикловая подсветка
    }
  }

  // Обслуживаем отсчёт отложенного выключения и импульсную программу CH6 каждый вызов — вне
  // троттлинга ниже, т.к. это времязависимая логика (см. комментарий про быстрый/медленный
  // уровни у DI3/4/5 выше; у CH6 вообще мс-точность, ей раз в секунду точно мало).
  ServiceLightsOffDelay();
  ServiceCh6Pulses();

  // DI3 — кнопка светомузыки: Peripherals_ButtonHeld() вернёт true один раз, когда кнопку
  // удержали дольше PERIPH_BUTTON_DEBOUNCE_MS (константа железа, не настройка) — тогда
  // переключаем CH3 (подсветку этой кнопки) в обратное состояние. Пока цикл идёт, то же
  // нажатие управляет и реальным CH6: выключили подсветку — программа 1 (сигнал "выкл"),
  // включили обратно — очередная программа 2..5 по кругу. Вне цикла CH6 не трогаем — он
  // управляет реальным оборудованием, а не просто подсветкой кнопки.
  if (Peripherals_ButtonHeld(lightMusicBtn, Peripherals_ReadInput(2), PERIPH_BUTTON_DEBOUNCE_MS)) {
    bool turningOn = !Peripherals_GetRelay(2); // CH3 сейчас выключена — значит, включаем
    Peripherals_SetRelay(2, turningOn); // CH3
    if (cycleActive) {
      if (turningOn) {
        PulseCh6NextOnProgram();
      } else {
        StartCh6Pulses(1); // выключили — программа 1
      }
    }
  }

  // DI4 — кнопка хромотерапии: та же механика, переключаем её собственную подсветку CH4.
  if (Peripherals_ButtonHeld(chromotherapyBtn, Peripherals_ReadInput(3), PERIPH_BUTTON_DEBOUNCE_MS)) {
    Peripherals_SetRelay(3, !Peripherals_GetRelay(3)); // CH4
  }

  // DI5 — кнопка нагрева: та же механика, переключаем её собственную подсветку CH5.
  if (Peripherals_ButtonHeld(heatingBtn, Peripherals_ReadInput(4), PERIPH_BUTTON_DEBOUNCE_MS)) {
    Peripherals_SetRelay(4, !Peripherals_GetRelay(4)); // CH5
  }

  // DI6 — кнопка понижения потока воздуха: та же механика антидребезга, но вместо реле
  // переключает по кругу di6LevelIndex (100/75/50/25%) — применяется в разгоне вентилятора
  // ниже как надстройка над AO1.max. Сброс на 100% — только при новом старте цикла (DI2).
  if (Peripherals_ButtonHeld(airflowDownBtn, Peripherals_ReadInput(5), PERIPH_BUTTON_DEBOUNCE_MS)) {
    di6LevelIndex = (di6LevelIndex + 1) % 4;
  }

  // Основная логика принятия решений — не чаще 1 раза в секунду.
  static unsigned long lastDecisionMs = 0;
  if (millis() - lastDecisionMs < 1000) return;
  lastDecisionMs = millis();

  // Пока цикл активен, CH7/CH8 (реальные исполнительные выходы) живьём следуют за CH4/CH5
  // (лампочки кнопок) — переключили кнопку во время цикла, через (максимум) секунду
  // поменяется и выход. CH6 сюда не входит — им целиком заведует импульсная программа
  // (ServiceCh6Pulses выше), а не подсветка CH3. При остановке цикла CH7/8 гасятся сразу в
  // блоке DI2 выше (CH6 получает программу 1 через StopCycle()).
  if (cycleActive) {
    Peripherals_SetRelay(6, Peripherals_GetRelay(3)); // CH7 = CH4
    Peripherals_SetRelay(7, Peripherals_GetRelay(4)); // CH8 = CH5

    // Наработка — секунды реальной работы цикла, полные минуты сразу видны на "Инфо"
    // (runtimeMinutesTotal — источник истины для отображения); в NVS попадёт при StopCycle.
    runtimeSecondsInMinute++;
    if (runtimeSecondsInMinute >= 60) {
      runtimeSecondsInMinute = 0;
      runtimeMinutesTotal++;
    }

    // Таймер самого цикла — считаем от logic_set.di2.cycle секунд, заведённых при старте.
    // Как только время вышло — это то же самое, что нажать Стоп или что человек ушёл, но
    // цикл в этом случае отработал полностью сам, поэтому и счётчик другой.
    if (cycleRemainingSec > 0) {
      cycleRemainingSec--;
      if (cycleRemainingSec == 0) StopCycle(true); // отработал сам, до конца
    }
  }

  // Ароматизация — досчитываем секунды импульса CH1, выключаем по достижении нуля.
  if (aromaRemainingSec > 0) {
    aromaRemainingSec--;
    if (aromaRemainingSec == 0) Peripherals_SetRelay(0, false); // CH1
  }

  // Разгон вентилятора — 3 фазы, в каждой мощность линейно растёт от значения предыдущей
  // фазы до своей цели за свою длительность в секундах (ровно столько раз сюда зайдём).
  if (fanPhase != 0) {
    fanPhaseElapsedSec++;
    bool phaseDone = fanPhaseElapsedSec >= fanPhaseDurationSec;
    uint8_t phasePct;
    if (phaseDone) {
      phasePct = fanPhaseTargetPct;
    } else {
      int16_t deltaPct = (int16_t)fanPhaseTargetPct - (int16_t)fanPhaseStartPct;
      phasePct = fanPhaseStartPct + (int16_t)(deltaPct * (int16_t)fanPhaseElapsedSec / (int16_t)fanPhaseDurationSec);
    }

    // AO1.max — ограничение выхода в процентах ОТ процента (см. комментарий у настройки в
    // settings.h): если фаза требует, скажем, 50% мощности, а AO1.max тоже 50%, реальный
    // выход должен быть на 50% от 50%, т.е. на 25%. DI6_LEVELS[di6LevelIndex] — ручная
    // надстройка НАД этим (ещё один процент от процента), которую можно менять кнопкой DI6
    // прямо во время цикла (см. ApplyFanOutputPercent).
    ApplyFanOutputPercent(phasePct);

    if (phaseDone) { // переходим к следующей фазе (или завершаем разгон)
      fanPhaseStartPct = fanPhaseTargetPct;
      fanPhaseElapsedSec = 0;
      if (fanPhase == 1) {
        fanPhase = 2;
        fanPhaseTargetPct = logic_set.ao1.f2_pwr;
        fanPhaseDurationSec = logic_set.ao1.f2_time;
      } else if (fanPhase == 2) {
        fanPhase = 3;
        fanPhaseTargetPct = logic_set.ao1.f3_pwr;
        fanPhaseDurationSec = logic_set.ao1.f3_time;
      } else {
        fanPhase = 0; // разгон завершён — выход остаётся на мощности 3-й фазы
      }
    }
  } else if (cycleActive) {
    // Разгон уже закончился, но цикл ещё идёт — держим устоявшуюся мощность
    // (fanPhaseTargetPct = цель последней, 3-й фазы) и продолжаем накладывать на неё DI6,
    // если его подвигали уже ПОСЛЕ разгона. Без этой ветки DI6 после разгона было бы
    // не к чему применить — ровно тот баг, который сейчас чиним.
    ApplyFanOutputPercent(fanPhaseTargetPct);
  }

  // Плавное гашение вентилятора после остановки цикла (StopCycle) — линейно от значения
  // ШИМ, которое было на момент остановки, до 0 за logic_set.ao1.delay_off секунд.
  if (fanRampDownActive) {
    fanRampDownElapsedSec++;
    if (fanRampDownElapsedSec >= fanRampDownDurationSec) {
      Peripherals_SetAnalogOut(0);
      fanRampDownActive = false;
    } else {
      uint8_t pwm = fanRampDownStartPwm -
                    (uint16_t)fanRampDownStartPwm * fanRampDownElapsedSec / fanRampDownDurationSec;
      Peripherals_SetAnalogOut(pwm);
    }
  }

  // Твой алгоритм — начиная отсюда.
}
