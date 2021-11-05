#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define degree_sysmbol 0xdf

volatile uint8_t timer0_total_ovf = 0;

char digits[] PROGMEM = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int hysteresis_range[] = {25, 30};

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

void ADC_Init()
{
    ADCSRA = 0x87; // freq/128
    ADMUX = 0x40;  // ADC channel 0
}

int ADC_Read(char channel)
{
    ADMUX = 0x40 | (channel & 0x07);
    ADCSRA |= (1 << ADSC);
    while (!(ADCSRA & (1 << ADIF)))
        ;
    ADCSRA |= (1 << ADIF);
    _delay_ms(1);
    return ADCW;
}

void lcd_write(char c)
{
    PORTC = c;
    PORTD |= (1 << 0);
    PORTD &= ~(1 << 1);
    PORTD |= (1 << 2);
    _delay_us(1);
    PORTD &= ~(1 << 2);
    _delay_ms(2);
}

void lcd_command(char c)
{
    PORTC = c;
    PORTD &= ~(1 << 0);
    PORTD &= ~(1 << 1);
    PORTD |= (1 << 2);
    _delay_us(1);
    PORTD &= ~(1 << 2);
    _delay_ms(2);
}

void lcd_init()
{
    _delay_ms(20);

    lcd_command(0x38);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);
    _delay_ms(2);
    lcd_command(0x80);
}

void update_display(char temp[])
{
    lcd_command(0x01);
    for (int i = 0; i < 16; i++)
    {
        lcd_write(temp[i]);
    }
}

int main(void)
{
    DDRC = 0xFF;

    DDRD = 0x0F;

    DDRA = 0x00;
    PORTA = 0x1E;

    lcd_init();
    ADC_Init();
    timer0_init();

    char hysteresis_str[16];
    char temp[16];
    float temp_celsius;

    while (1)
    {
        if (temp_celsius > hysteresis_range[1])
        {
            PORTD |= (1 << 3);
        }

        if (temp_celsius < hysteresis_range[0])
        {
            PORTD &= ~(1 << 3);
        }

        if (!(PINA & (1 << 1)))
        {
            hysteresis_range[1] = hysteresis_range[1] + 1;
            sprintf(hysteresis_str, "hyst upper: %d%cC  ", (int)hysteresis_range[1], degree_sysmbol);
            update_display(hysteresis_str);
            _delay_ms(500);
            while (!(PINA & (1 << 1)));
        }

        if (!(PINA & (1 << 2)))
        {
            hysteresis_range[1]--;
            sprintf(hysteresis_str, "hyst upper: %d%cC  ", (int)hysteresis_range[1], degree_sysmbol);
            update_display(hysteresis_str);
            _delay_ms(500);
            while (!(PINA & (1 << 2)));
        }

        if (!(PINA & (1 << 3)))
        {
            hysteresis_range[0]++;
            sprintf(hysteresis_str, "hyst upper: %d%cC  ", (int)hysteresis_range[0], degree_sysmbol);
            update_display(hysteresis_str);
            _delay_ms(500);
            while (!(PINA & (1 << 3)));
        }

        if (!(PINA & (1 << 4)))
        {
            hysteresis_range[0]--;
            sprintf(hysteresis_str, "hyst upper: %d%cC  ", (int)hysteresis_range[0], degree_sysmbol);
            update_display(hysteresis_str);
            _delay_ms(500);
            while (!(PINA & (1 << 4)));
        }

        if (timer0_total_ovf >= 61)
        {
            temp_celsius = (ADC_Read(0) * 4.88); // Step size with Vref=5V:  5/1023 = 4.88 mV.
            temp_celsius = (temp_celsius / 10.00);
            sprintf(temp, "Temperature: %d%cC  ", (int)temp_celsius, degree_sysmbol);
            update_display(temp);
            _delay_ms(500);
            TCNT0 = 0;
            timer0_total_ovf = 0;
        }
    }
}

ISR(TIMER0_OVF_vect)
{
    timer0_total_ovf++;
}