/*
	Button - a small library for Arduino to handle button debouncing
	
	MIT licensed.
*/

#ifndef Button_h
#define Button_h
#include "Arduino.h"

class Button
{
	public:
		Button(uint8_t pin, uint16_t debounce_ms, uint8_t pinMode = INPUT_PULLUP);
		void begin();
		bool read();
		bool toggled();
		bool pressed();
		bool released();
		bool has_changed();
        bool longPress(uint16_t delay_ms);
		
		const static bool PRESSED = LOW;
		const static bool RELEASED = HIGH;
	
	private:
		uint8_t  _pin;
		uint16_t _delay;
        uint16_t _pin_mode;
		bool     _state;
		uint32_t _ignore_until;
		bool     _has_changed;
        uint32_t _pressed_since;
};

#endif