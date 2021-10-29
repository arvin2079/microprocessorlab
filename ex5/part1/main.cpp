#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

volatile uint8_t timer0_total_ovf = 0;
volatile uint8_t cycle_counter = 0;

char cycles[] PROGMEM = {0x09, 0x05, 0x06, 0x0A};

void timer0_init()
{
    // prescaler = clk/64
    TCCR0 |= (1 << CS01) | (1 << CS00);

    // initialization of timer 1 register
    TCNT0 = 0;

    // enabling timer0 intrrupt
    TIMSK |= (1 << TOIE0);
    sei();
}

int main(void)
{
    timer0_init();

    DDRA = 0x0F;
    PORTA = 0x00;

    DDRC = 0x00;
    PORTC = 0x04;

    while (1)
    {

        // 1ms
        if ((timer0_total_ovf >= 2) & !(PINC & (1 << 2)))
        {

            TCNT0 = 0;
            while (TCNT0 <= 113);

            PORTA = pgm_read_byte(&(cycles[cycle_counter]));

            TCNT0 = 0;
            timer0_total_ovf = 0;

            cycle_counter++;

            if (cycle_counter >= 4)
            {
                cycle_counter = 0;
            }

        }
    }
}

ISR(TIMER0_OVF_vect)
{
    timer0_total_ovf++;
}