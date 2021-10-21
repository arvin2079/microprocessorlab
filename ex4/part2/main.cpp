#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

int s = 0; 
int m = 0; 
int h = 0; 

char digits[] PROGMEM = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

void lcd_write(char c) {
    PORTA = c;
    PORTB |= 0x03;
    _delay_ms(5);
    PORTB &= ~0x01;
}

void lcd_command(char c) {
    PORTA = c;
	PORTB &= ~0x02;
	PORTB |= 0x01;
	_delay_ms(5);
	PORTB &= ~0x01;
}

void lcd_init() {
    lcd_command(0x38);
	lcd_command(0x06);
	lcd_command(0x0E);
	lcd_command(0x01);
	lcd_command(0x80);
}

void timer2_init()
{
    // prescaler = clk/8
    TCCR2 |= (1 << CS22);

    // initialization of timer 1 register
    TCNT2 = 0;
}


void realtime_clock_trigger() {
    if (++s >= 60) {
        s = 0;
        if (++m >= 60) {
            m = 0;
            if (++h >= 24) {
                h = 0;
            }
        }
    }
}

void update_display() {
    lcd_command(0x01);
    if (h < 10) {
        lcd_write('0');
        lcd_write(pgm_read_byte(&(digits[h])));
    } 
    
    else {
        lcd_write(pgm_read_byte(&(digits[h / 10])));
        lcd_write(pgm_read_byte(&(digits[h % 10])));
    }

    lcd_write(':');

    if (m < 10) {
        lcd_write('0');
        lcd_write(pgm_read_byte(&(digits[m])));
    } 
    
    else {
        lcd_write(pgm_read_byte(&(digits[m / 10])));
        lcd_write(pgm_read_byte(&(digits[m % 10])));
    }

    lcd_write(':');

    if (s < 10) {
        lcd_write('0');
        lcd_write(pgm_read_byte(&(digits[s])));
    } 
    
    else {
        lcd_write(pgm_read_byte(&(digits[s / 10])));
        lcd_write(pgm_read_byte(&(digits[s % 10])));
    }
}


int main() {

    DDRA = 0xff;
    DDRB = 0x03;

    lcd_init();
    timer2_init();

    int clock_trigger_counter = 0;
    
    while(1){

        // 1 second 125 triggers of 125 clock cycle
        if (TCNT2 >= 125) {
            clock_trigger_counter ++;
            TCNT2 = 0;
        }

        if(clock_trigger_counter >= 125) {
            realtime_clock_trigger();
            update_display();
            clock_trigger_counter = 0;
        }
    }
}
