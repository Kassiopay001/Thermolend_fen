#pragma once

#include <Arduino.h>

#define PCA9554_REG_INPUT 0x00
#define PCA9554_REG_OUTPUT 0x01
#define PCA9554_REG_POLARITY 0x02
#define PCA9554_REG_CONFIG 0x03

class Pca9554 {
 public:
  explicit Pca9554(uint8_t address);

  bool isPresent();
  void begin(uint8_t outputMask = 0x00);

  void writePin(uint8_t pin, bool state);
  bool readPin(uint8_t pin);

  void writeAll(uint8_t mask);
  uint8_t readAll();

 private:
  uint8_t _address;
  uint8_t _outputState;

  void writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);
};
