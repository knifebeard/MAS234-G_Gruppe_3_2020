/*
 * Mega168_PWM_LED.cpp
 *
 * Created: 23.10.2020 10.06.11
 * Author : Snorre
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

double dutyCycle = 0;

int main(void)
{
	DDRD = (1 << PD6); //Enable pin PD6 for output.
	
	TCCR0A = (1 << COM1A1) | (1 << WGM00) | (1 << WGM01); //WGM00 and WGM01 activate Fast PWM Mode. COM1A1 resets timer OC1A on compare match. 
	TIMSK0 = (1 << TOIE0); //Enables interrupt on overflow for timer OC1A. Timer Interrupt Mask Register. 
	
	OCR0A = (dutyCycle/100)*255; //Sets the value that the timer will compare to. TOP value. 
	
	sei(); //Enables global interrupt 
	
	TCCR0B = (1 << CS00) | (1 << CS01); //TCCR0B determines clock source for timers. Set CS00 and CS01 for IO clock to be 1/64 of internal clock.
	
	while(1)
	{
		_delay_ms(10);
		
		dutyCycle += 1;
		
		if (dutyCycle > 100)
		{
			dutyCycle = 0;
		}
	}
	return(0);
}

ISR(TIMER0_OVF_vect)
{
	OCR0A = (dutyCycle/100)*255;
}