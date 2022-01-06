//deBounce.h

#ifndef DEBOUNCE_MUNT_H
#define DEBOUNCE_MUNT_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class DebounceButton {
public:
  DebounceButton(int b_pin, unsigned long debounceTime,unsigned char inputMode);
  void setupButton();
  int read();
  bool longPress(unsigned long duration);
private:
  int _b_pin;
  unsigned long _debounceTime;
  unsigned char _inputMode;
  unsigned long _debounce;
  unsigned long _pressDuration;
};
#endif