#define F_CPU 1000000UL                                         /* 1MHz */
#define USART_BAUDRATE 4800                                     /* bit or baud per second */
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) /* BAUD_PRESCALE = 51 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#define degree_sysmbol 0xdf


char cycles[] PROGMEM = {0x09, 0x05, 0x06, 0x0A};

volatile uint8_t motor_step_follower = 0;


void UART_init() {

    // RX complete intrrupt and TX complete intrrupt enable
    UCSRB |= (1 << RXEN) | (1 << TXEN) | (1 << RXCIE) | (1 << TXCIE);

    // 8 bit data unit size
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);

    UBRRH = (BAUD_PRESCALE >> 8);
    UBRRL = BAUD_PRESCALE;
}

void UART_write_char(char data) {
    // wait while register is free
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

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

char motor_next_step() {
    return pgm_read_byte(&(cycles[(++motor_step_follower) % 4]));
}

void run_stepper_motor(uint8_t steps_no) {
    for (uint8_t i = 0; i < steps_no; i++) {
        PORTB = motor_next_step();
        _delay_ms(500);
    }
}


int main(void) {
    DDRB = 0x0F;
    PORTB = 0x00;

    PORTD = 0x04;

    UART_init();
    ADC_Init();

    sei();

    char temp[16];
    float temp_celsius;

    while (1) {
        if (!(PIND & (1 << 2))) {
            temp_celsius = (ADC_Read(0)*4.88); // Step size with Vref=5V:  5/1023 = 4.88 mV. 
            temp_celsius = (temp_celsius / 10.00);
            sprintf(temp,"Temp: %d%cC", (int)temp_celsius, degree_sysmbol);
            for (int i = 0; i < 16; i++)
            {
                UART_write_char(temp[i]);
            }
            while(!(PIND & (1 << 2)));
        }
    }
}


ISR(USART_RXC_vect) {
    run_stepper_motor(UDR - '0');
}