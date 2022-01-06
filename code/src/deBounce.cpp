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
  this->_pressDuration = -1;
}

void DebounceButton::setupButton() {
  pinMode(_b_pin, _inputMode);
}

int DebounceButton::read()    {

  /*if(_debounceTime!=0)
  {*/
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
  //}
  /*else 
  {
    if(digitalRead(_b_pin) == HIGH)
    {
      _debounce = millis();
      return HIGH;
    }
    if(millis()- _debounce < 1000)
    {
      return HIGH;
    }
    return LOW;
  }*/
}

bool DebounceButton::longPress(unsigned long duration){
  if(this->read()==HIGH)
  {
    if(_pressDuration==0){
      //initiate a long press
      _pressDuration = millis();
      return false;
    }
    if( millis() - _pressDuration >= duration)
    {
      _pressDuration = 0;
      return true;
    }
    return false;
  }
  else {
    this->_pressDuration = 0;
    return false;
  }
}
