#include <avr/io.h>
#include <util/delay.h>

#define F_CPU = 100000UL;

// 16.11.1
void initiatePWM() {
	// WGM12 and WGM10 fro Fast PWM, 8-bit
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // Clear OC1A/OC1B on compare match, set OC1A/OC1B at BOTTOM (non-inverting mode)
	TCCR1B |= (1 << WGM12);
	//Data Direction Register (DDR) bit corresponding to the OC1A or OC1B pin must be set in order to enable the output driver
	DDRB = (1 << B2) | (1 << B1);
}

void main () {

	DDRC = 0xff;
	DDRB = 0xff;
	DDRD = 0xff;
	PORTC = 0b000000000;
	PORTD = 0b000000000;
	uint8_t minuten = 0;
	uint8_t stunden = 0;
	
	while(1) {

		PORTC = (minuten & 0b11111111);
		PORTD = (stunden & 0b11111111);

		minuten++;
		if (minuten >= 60) {
			minuten = 0;
			stunden++;

			if (stunden >= 24) {
				stunden = 0;
			}
		}

		PORTB |= (1 << PB1);
		_delay_ms(10);
		PORTB &= ~(1 << PB1);
		_delay_ms(90);

		PORTB |= (1 << PB2);
		_delay_ms(10);
		PORTB &= ~(1 << PB2);
		_delay_ms(90);
	}
}
