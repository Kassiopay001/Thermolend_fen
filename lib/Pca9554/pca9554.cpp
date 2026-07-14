#include "pca9554.h"

#include <Wire.h>

Pca9554::Pca9554(uint8_t address) : _address(address), _outputState(0) {}

bool Pca9554::isPresent() {
  Wire.beginTransmission(_address);
  return Wire.endTransmission() == 0;
}

void Pca9554::begin(uint8_t outputMask) {
  writeRegister(PCA9554_REG_CONFIG, 0x00);
  _outputState = outputMask;
  writeRegister(PCA9554_REG_OUTPUT, _outputState);
}

void Pca9554::writePin(uint8_t pin, bool state) {
  if (pin > 7) return;
  if (state) {
    _outputState |= (1 << pin);
  } else {
    _outputState &= ~(1 << pin);
  }
  writeRegister(PCA9554_REG_OUTPUT, _outputState);
}

bool Pca9554::readPin(uint8_t pin) {
  if (pin > 7) return false;
  return readRegister(PCA9554_REG_INPUT) & (1 << pin);
}

void Pca9554::writeAll(uint8_t mask) {
  _outputState = mask;
  writeRegister(PCA9554_REG_OUTPUT, _outputState);
}

uint8_t Pca9554::readAll() {
  return readRegister(PCA9554_REG_OUTPUT);
}

void Pca9554::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t Pca9554::readRegister(uint8_t reg) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((int)_address, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}
