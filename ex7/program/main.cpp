#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#define LcdDataBus PORTA
#define LcdControlBus PORTB
#define keypadPort PORTC
#define keypadPin PINC

#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2

// username password variables
char true_id[] = "1111";
char true_password[] = "1111";

char user_id[4];
char user_password[4];

int credential_counter;

bool id_entered = false;
bool access_granted = false;

//keypad variables
char keypad_shift[4] PROGMEM  = {0xFE, 0xFD, 0xFB, 0xF7};
char keypad_layout[16] PROGMEM  = {'7', '8', '9', '/', '4', '5', '6', '*', '1', '2', '3', '-', 'C', '0', '=', '+'};

//for test purposes
char layout[6]  = {'0', '1', '2', '3', '4', '5'};


void lcd_write_char(char c)
{
    LcdDataBus = c;
    LcdControlBus |= (1 << LCD_RS);
    LcdControlBus &= ~(1 << LCD_RW);
    LcdControlBus |= (1 << LCD_EN);
    _delay_us(1);
    LcdControlBus &= ~(1 << LCD_EN);
    _delay_ms(1);
}

void lcd_command(char c)
{
    LcdDataBus = c;
    LcdControlBus &= ~(1 << LCD_RS);
    LcdControlBus &= ~(1 << LCD_RW);
    LcdControlBus |= (1 << LCD_EN);
    _delay_us(1);
    LcdControlBus &= ~(1 << LCD_EN);
    _delay_ms(3);
}

void lcd_write_string(char *c)
{
    int i;
    for (i = 0; c[i] != 0; i++)
    {
        lcd_write_char(c[i]);
    }
}

void lcd_write_at_xy (char row, char pos, char *c)
{
	if (row == 0 && pos<16)
	lcd_command((pos & 0x0F)|0x80);	
	else if (row == 1 && pos<16)
	lcd_command((pos & 0x0F)|0xC0);
	lcd_write_string(c);		
}

void lcd_next_line() {
    lcd_command(0xC0);
}

void lcd_init()
{
    DDRA = 0xFF;
    DDRB = 0x07;
    _delay_ms(20);
    lcd_command(0x38);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);
    _delay_ms(2);
    lcd_command(0x80);
}

void lcd_cursor_at_first() {
    lcd_command(0x02);
}

void lcd_clear()
{
    lcd_command(0x01);
    lcd_command(0x80);
}

void intrrupts_init() {
    GICR = 1<<INT0;
    MCUCR |= (1 << ISC01);
    sei();
}


char get_selected_key() {
    int row=0, col=-1, position=-1;
    for (row=0 ; row<4 ; row++) {
        keypadPort = pgm_read_byte(&keypad_shift[row]);
        if(!(keypadPin & (1 << 4))) {col=0;}
        if(!(keypadPin & (1 << 5))) {col=1;}
        if(!(keypadPin & (1 << 6))) {col=2;}
        if(!(keypadPin & (1 << 7))) {col=3;}
        if(col != -1) {
            position = row * 4 + col;
            keypadPort = 0xF0;
            return pgm_read_byte(&keypad_layout[position]);
            // lcd_write_char(pgm_read_byte(&keypad_layout[position]));
            // col = -1;
        }
        _delay_ms(20);
    }
    keypadPort = 0xF0;
}


int main(void) {
    intrrupts_init();
    lcd_init();

    DDRC = 0x0F;
    DDRD |= (1 << PD3) | (1 << PD4);
    keypadPort = 0xF0;

    lcd_clear();
    lcd_write_string("enter your id:"); 
    lcd_next_line();

    credential_counter = -1;

    while (1) {}
}

void check_credentials() {
    // lcd_clear();
    // lcd_write_string(user_password);
    // lcd_next_line();
    // lcd_write_string(true_password);
    // _delay_ms(3000);
    access_granted = true;
    for(int i=0 ; i<strlen(true_password) ; i++) {
        access_granted &= user_password[i] == true_password[i];
    }

    for(int i=0 ; i<strlen(true_id) ; i++) {
        access_granted &= user_id[i] == true_id[i];
    }
}

void authenticate() {
    if (!id_entered) {
        if (credential_counter == -1) { 
            credential_counter = 0;
        }
        char selected_key = get_selected_key();
        user_id[credential_counter++] = selected_key;
        lcd_write_char(selected_key);
        if(credential_counter >= strlen(true_id)) {
            lcd_clear();
            lcd_write_string("enter password:"); 
            lcd_next_line();
            id_entered = true;
            credential_counter = -1; 
        }
    } else {
        if (credential_counter == -1) { 
            credential_counter = 0; 
        }
        char selected_key = get_selected_key();
        user_password[credential_counter++] = selected_key;
        lcd_write_char('*');
        if(credential_counter >= strlen(true_password)) { 
            check_credentials();
            if(access_granted) {
                PORTD |= (1 << PD3);
                lcd_clear();
                lcd_write_string("Access approved!");
                _delay_ms(1000);
                PORTD &= ~(1 << PD3);
                lcd_clear();
            } else {
                PORTD |= (1 << PD4);
                lcd_clear();
                lcd_write_string("Access denied!");
                _delay_ms(1000);
                PORTD &= ~(1 << PD4);
                id_entered = false;
                lcd_clear();
                lcd_write_string("enter your id:"); 
                lcd_next_line();
            }
            credential_counter = -1; 
        }
    }
}

ISR(INT0_vect)
{
    if (!access_granted) {
        authenticate();
    } else {

    }
    // char selected_key = get_selected_key();
}
