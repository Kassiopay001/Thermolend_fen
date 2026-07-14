#include <Arduino.h>

#include "logic.h"
#include "peripherals.h"
#include "settings.h"
#include "web.h"

void setup() {
  Serial.begin(115200);

  Settings_Init();
  Peripherals_Init();
  Logic_Init();
  Web_Init();
  Web_OtaBegin();
}

void loop() {
  Peripherals_Loop();
  Logic_Update();
  Web_Loop();
}
