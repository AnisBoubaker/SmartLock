//deBounce.cpp

#include "deBounce.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
DebounceButton::DebounceButton(int b_pin, unsigned long debounceTime,unsigned char inputMode)
{
  _b_pin = b_pin;
  _debounceTime = debounceTime;
  _inputMode=inputMode;
  pinMode(_b_pin, _inputMode);
  //digitalWrite(_b_pin, HIGH);
}

void DebounceButton::setupButton() {
  pinMode(_b_pin, _inputMode);
}

int DebounceButton::read()    {

  if (millis()-_debounce < _debounceTime){
    return LOW;
  }
  if  (digitalRead(_b_pin) == HIGH){
    if (millis()-_debounce > _debounceTime){
      _debounce=millis();
      return HIGH;
    }
    else {
      return LOW;
    }
  }
  return LOW;
}

