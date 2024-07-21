#include <Arduino.h>


#ifndef UltimateDebounce_H
#define UltimateDebounce_H


class UltimateDebounce
{
	public:
		// Constructor - Pass HW Pin for Button to debounce
		UltimateDebounce(uint8_t buttonPin);		// Defaults to Active HIGH
		
		// Constructor - Pass HW Pin for Button to debounce, Active state (0=Active Low, 1 = Active High)
		UltimateDebounce(uint8_t buttonPin, uint8_t activeLevel);

		void update(void);
 
		uint8_t is_up(void);
		uint8_t is_down(void);
		uint8_t is_pressed(void);
		uint8_t is_released(void);
	
	protected:
	private:
		uint8_t  _histSize=0;
		uint16_t _button_history16=0;
		uint8_t  _button_history=0;
		// uint8_t _press_count=0;
		// uint8_t _release_count=0;
		uint8_t _button=0;
		uint8_t _active = 0;

		uint8_t read_button(void);

};

class UltimateDebounce16		// Used for 16 Bits of history - better noise exclusion, slower response
{
	public:
		// Constructor - Pass HW Pin for Button to debounce
		UltimateDebounce16(uint8_t buttonPin);		// Defaults to Active HIGH
		
		// Constructor - Pass HW Pin for Button to debounce, Active state (0=Active Low, 1 = Active High)
		UltimateDebounce16(uint8_t buttonPin, uint8_t activeLevel);
		

		void update(void);
 
		uint8_t is_up(void);
		uint8_t is_down(void);
		uint8_t is_pressed(void);
		uint8_t is_released(void);

	
	protected:
	private:
		
		uint16_t _button_history=0;			// 16 Bit History
		uint8_t _button=0;
		uint8_t _active = 0;

		uint8_t read_button(void);

};

#endif	// UltimateDebounce_H