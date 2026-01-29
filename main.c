#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

// ===== Neue Pinbelegung =====
#define BUTTON1 (1 << PD5)
#define BUTTON2 (1 << PD6)
#define BUTTON3 (1 << PD7)

#define LED_H_1 (1 << PD4)
#define LED_H_2 (1 << PD3)
#define LED_H_3 (1 << PD2)
#define LED_H_4 (1 << PD1)
#define LED_H_5 (1 << PD0)

#define LED_M_MASK 0x3F

// MESS-PIN DEFINITION
#define SCOPE_PIN (1 << PB0)

volatile uint8_t seconds = 0;
volatile uint8_t minutes = 20;
volatile uint8_t hours = 5;

volatile uint8_t brightness_level = 0;
const uint8_t brightness_levels[] = {255, 248, 156, 0};

void setup_timer2() {
    ASSR |= (1 << AS2);
    TCCR2A = 0;
    TCCR2B = (1 << CS22) | (1 << CS20);
    TIMSK2 = (1 << TOIE2);
    while (ASSR & ((1 << TCR2AUB) | (1 << TCR2BUB))) { }
}

void setup_timer1() {
    TCCR1A = (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A = brightness_levels[brightness_level];
    OCR1B = brightness_levels[brightness_level];
}

void setup_ports() {
    DDRD |= (LED_H_1 | LED_H_2 | LED_H_3 | LED_H_4 | LED_H_5);
    DDRC |= LED_M_MASK;
    DDRB |= (1 << PB1) | (1 << PB2);
    
    // ÄNDERUNG: PB0 als Ausgang für das Oszilloskop
    DDRB |= SCOPE_PIN; 

    PRR |= (1 << PRADC) | (1 << PRUSART0) | (1 << PRSPI) |
           (1 << PRTIM0) | (1 << PRTWI);

    DDRD &= ~(BUTTON1 | BUTTON2 | BUTTON3);
    PORTD |= (BUTTON1 | BUTTON2 | BUTTON3);

    PORTD &= ~(LED_H_1 | LED_H_2 | LED_H_3 | LED_H_4 | LED_H_5);
    PORTC &= ~LED_M_MASK;

    OCR1A = brightness_levels[brightness_level];
    OCR1B = brightness_levels[brightness_level];
}

void setup_interrupts() {
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23);
}

void update_display() {
    PORTD &= ~(LED_H_1 | LED_H_2 | LED_H_3 | LED_H_4 | LED_H_5);
    if (hours & (1 << 0)) PORTD |= LED_H_1;
    if (hours & (1 << 1)) PORTD |= LED_H_2;
    if (hours & (1 << 2)) PORTD |= LED_H_3;
    if (hours & (1 << 3)) PORTD |= LED_H_4;
    if (hours & (1 << 4)) PORTD |= LED_H_5;

    PORTC &= ~LED_M_MASK;
    PORTC |= (minutes & LED_M_MASK);
}

ISR(TIMER2_OVF_vect) {
    // ÄNDERUNG: PB0 toggeln (Umschalten 0 <-> 1)
    PINB |= SCOPE_PIN; 
    _delay_us(100);      // 100 Mikrosekunden warten
    PORTB &= ~SCOPE_PIN; // Pin wieder auf LOW

    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) hours = 0;
        }
    }
    update_display();
}

ISR(PCINT2_vect) {
    static uint8_t prev = 0xFF;
    uint8_t curr = PIND;

    if ( (prev & BUTTON1) && !(curr & BUTTON1) ) {
        _delay_ms(5);
        if (!(PIND & BUTTON1)) {
            minutes++;
            seconds = 0;
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                if (hours >= 24) hours = 0;
            }
            update_display();
        }
    }

    if ( (prev & BUTTON2) && !(curr & BUTTON2) ) {
        _delay_ms(5);
        if (!(PIND & BUTTON2)) {
            hours++;
            if (hours >= 24) hours = 0;
            update_display();
        }
    }

    if ( (prev & BUTTON3) && !(curr & BUTTON3) ) {
        _delay_ms(5);
        if (!(PIND & BUTTON3)) {
            brightness_level = (brightness_level + 1) % 4;
            OCR1A = brightness_levels[brightness_level];
            OCR1B = brightness_levels[brightness_level];
        }
    }
    prev = curr;
}

void enter_sleep() {
    if (OCR1A == 255 && OCR1B == 255) {
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_enable();
        sleep_cpu();
    } else {
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
    }
    sleep_disable();
}

int main() {
    setup_ports();
    setup_timer2();
    setup_timer1();
    setup_interrupts();

    sei();

    brightness_level = 2;
    OCR1A = brightness_levels[brightness_level];
    OCR1B = brightness_levels[brightness_level];
    update_display();

    while (1) {
        enter_sleep();
    }
    return 0;
}