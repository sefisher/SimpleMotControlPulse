// ** Library for SW Debounce based on method from:
// 		https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/#more-180185  

#include <Arduino.h>
#include <UltimateDebounce.h>

#define MASK8   	0b11000111		// Mask 0's are "don't care"

#define RELEASE8 	0b11000000
#define DOWN8 		0b11111111
#define UP8 		0b00000000
#define PRESS8		0b00000111

#define CLEAR_HIST8 0b00000000
#define SET_HIST8	0b11111111

// How to impliment 16 bit option Without being a seperate Class, or mess of IF statements?

#define MASK16  		0b1110000000011111		// Mask 0's are "don't care"			

#define RELEASE16 		0b1110000000000000
#define DOWN16 			0b1111111111111111
#define UP16 			0b0000000000000000
#define PRESS16			0b0001111111111111

#define CLEAR_HIST16 	0b0000000000000000
#define SET_HIST16		0b1111111111111111

// 8 BIT Class    (16 Bit Below)
///////////////////////////////////////////////////////////////////////////////
// Constructors
UltimateDebounce::UltimateDebounce(uint8_t buttonPin)  // Set pin to track
{
	_button = buttonPin;
	_active = HIGH;
}

UltimateDebounce::UltimateDebounce(uint8_t buttonPin, uint8_t activeLevel)  // Set pin to track & Pin Active Setting
{
	_button = buttonPin;
	_active = activeLevel;			// LOW or HIGH
}


// FUNCTIONS

uint8_t UltimateDebounce::read_button(void){   
	
	uint8_t result = false;
	if(_active == LOW && digitalRead(_button) == LOW){
		result = true;
	}
	else if(_active == HIGH && digitalRead(_button) == HIGH){
		result = true;
	}
    return result;
}

void UltimateDebounce::update(void){
    _button_history = (_button_history << 1);
    _button_history |= read_button();
}

uint8_t UltimateDebounce::is_pressed(void){
    uint8_t _pressed = 0; 
	
	if ((_button_history & MASK8) == PRESS8){ 
		_pressed = 1;
		_button_history = SET_HIST8;
	}
    return _pressed;
}

uint8_t UltimateDebounce::is_released(void){
    uint8_t _released = 0;   
		
	if ((MASK8 & _button_history) == RELEASE8)
	{ 
		_released = 1;
		_button_history = CLEAR_HIST8;
	}
    return _released;
}

uint8_t UltimateDebounce::is_down(void){

	return (_button_history == DOWN8);
}

uint8_t UltimateDebounce::is_up(void){

	return (_button_history == UP8);			// Return a 1 if History matches UP condition Pattern
}

// END 8 Bit Class 
////////////////////////////////////////////////////////////////////////////////////

 

// 16 BIT Class  
///////////////////////////////////////////////////////////////////////////////
// Constructors
UltimateDebounce16::UltimateDebounce16(uint8_t buttonPin)  // Set pin to track
{
    _button = buttonPin;
	_active = HIGH;
}

UltimateDebounce16::UltimateDebounce16(uint8_t buttonPin, uint8_t activeLevel)  // Set pin to track & Pin Active Setting
{
    _button = buttonPin;
	_active = activeLevel;			// LOW or HIGH
	
}


// FUNCTIONS

uint8_t UltimateDebounce16::read_button(void){   					// Same as 8 Bit Class
	
	uint8_t result = false;
	if(_active == LOW && digitalRead(_button) == LOW){
		result = true;
	}
	else if(_active == HIGH && digitalRead(_button) == HIGH){
		result = true;
	}
    return result;
}

void UltimateDebounce16::update(void){
    _button_history = (_button_history << 1);
    _button_history |= read_button();
}

uint8_t UltimateDebounce16::is_pressed(void){
    uint8_t _pressed = 0; 
	
	if ((_button_history & MASK16) == PRESS16){ 
		_pressed = 1;
		_button_history = SET_HIST16;
	}
    return _pressed;
}

uint8_t UltimateDebounce16::is_released(void){
    uint8_t _released = 0;   
		
	if ((MASK16 & _button_history) == RELEASE16)
	{ 
		_released = 1;
		_button_history = CLEAR_HIST16;
	}
    return _released;
}

uint8_t UltimateDebounce16::is_down(void){

	return (_button_history == DOWN16);			// Return a 1 if History matches DOWN condition Pattern
}

uint8_t UltimateDebounce16::is_up(void){

	return (_button_history == UP16);			// Return a 1 if History matches UP condition Pattern
}

// END 16 Bit Class 
////////////////////////////////////////////////////////////////////////////////////