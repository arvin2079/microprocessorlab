#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <math.h>

volatile uint8_t timer0_total_ovf = 0;
volatile int cycle_counter = 0;
volatile bool anticlockwise = false;

volatile float time_treshhold = 1;
const float time_step = 0.03125;
const int max_time = 10;

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

    DDRC = 0x01;

    while (1)
    {
        anticlockwise = (sin(time_treshhold * M_PI / 3) < 0);

        if (anticlockwise)
        {
            PORTC |= (1 << 0);
        }
        else
        {
            PORTC &= ~(1 << 0);
        }

        if (timer0_total_ovf >= max_time * fabs(sin(time_treshhold * M_PI / 3)))
        {

            PORTA = pgm_read_byte(&(cycles[cycle_counter]));

            timer0_total_ovf = 0;


            anticlockwise ? cycle_counter-- : cycle_counter++;

            if (cycle_counter >= 4)
            {
                cycle_counter = 0;
            }

            if (cycle_counter <= -1)
            {
                cycle_counter = 3;
            }

            time_treshhold += time_step;

            if (time_treshhold >= 6)
            {
                time_treshhold = 0;
            }
        }
    }
}

ISR(TIMER0_OVF_vect)
{
    timer0_total_ovf++;
}