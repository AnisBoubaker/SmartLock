/*
	Button - a small library for Arduino to handle button debouncing
	
	MIT licensed.
*/

#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin, uint16_t debounce_ms, uint8_t pinMode)
:  _pin(pin)
,  _delay(debounce_ms)
, _pin_mode(pinMode)
,  _state(HIGH)
,  _ignore_until(0)
,  _has_changed(false)
,  _pressed_since(0)
{
}

void Button::begin()
{
	pinMode(_pin, _pin_mode);
}

// 
// public methods
// 

bool Button::read()
{
	// ignore pin changes until after this delay time
	if (_ignore_until > millis())
	{
		// ignore any changes during this period
	}
	
	// pin has changed 
	else if (digitalRead(_pin) != _state)
	{
		_ignore_until = millis() + _delay;
		_state = !_state;
		_has_changed = true;
	}
	
	return _state;
}

// has the button been toggled from on -> off, or vice versa
bool Button::toggled()
{
	read();
	return has_changed();
}

bool Button::longPress(uint16_t delay_ms)
{
    read();
    if(!_state){
        _pressed_since = 0;
        return false;
    } 
    if(_pressed_since==0)
    {
        _pressed_since = millis();
        return false;
    }
    if(millis() - _pressed_since >= delay_ms)
    {
        _pressed_since = 0;
        return true;
    }
    return false;
}


// mostly internal, tells you if a button has changed after calling the read() function
bool Button::has_changed()
{
	if (_has_changed)
	{
		_has_changed = false;
		return true;
	}
	return false;
}

// has the button gone from off -> on
bool Button::pressed()
{
	return (read() == PRESSED && has_changed());
}

// has the button gone from on -> off
bool Button::released()
{
	return (read() == RELEASED && has_changed());
}