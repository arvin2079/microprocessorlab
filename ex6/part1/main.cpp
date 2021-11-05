#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define degree_sysmbol 0xdf

char digits[] PROGMEM = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

void ADC_Init()
{
    DDRA = 0x00;
    ADCSRA = 0x87; // freq/128
    ADMUX = 0x40; // ADC channel 0
}

int ADC_Read(char channel)
{
    ADMUX = 0x40 | (channel & 0x07);
    ADCSRA |= (1 << ADSC);
    while (!(ADCSRA & (1 << ADIF)));
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
    DDRC = 0xFF;
    DDRD = 0x07;
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
    lcd_init();
    ADC_Init();

    char temp[16];
    float temp_celsius;

    while (1)
    {
        temp_celsius = (ADC_Read(0)*4.88); // Step size with Vref=5V:  5/1023 = 4.88 mV. 
        temp_celsius = (temp_celsius / 10.00);
        sprintf(temp,"Temperature: %d%cC  ", (int)temp_celsius, degree_sysmbol);
        update_display(temp);
        _delay_ms(500);
    }
}

// dont forget to connect avr avcc to power