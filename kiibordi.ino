#include <LiquidCrystal.h>

#define RS     0
#define E      1
#define INTRP1 2
#define INTRP2 3
#define CLK    4
#define DATA   5
#define OE     6
#define DB4    14
#define DB5    15
#define DB6    16
#define DB7    17
#define COL_LEN    16
#define ROW_LEN    2
#define BACKSPACE  '\8'

const char CHAR_MAP_1[] = "ABCDEFGHIJKLMNOP";
const char CHAR_MAP_2[] = "QRSTUVWXYZ.!? \8\n";

volatile int keyboard_col = 0;
volatile int lcd_col = 0;
volatile char buffer_row2[COL_LEN + 1] = {0};
LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);


void isr_row2() {
	noInterrupts();
	char c = CHAR_MAP_2[keyboard_col];
	if (c == BACKSPACE) {
      	if (lcd_col == 0)
          	goto end;
    	buffer_row2[--lcd_col] = ' ';
    	lcd.setCursor(lcd_col, 1);
      	lcd.write(' ');
      	lcd.setCursor(lcd_col, 1);
    } else if (c == '\n') {
      	/* write second row to first row */
      	lcd.clear();
      	lcd.home();
      	lcd.print((char *)buffer_row2);
      	lcd.setCursor(0, 1);
		/* clear buffer */
      	for (int i = 0; i < COL_LEN; i++)
			buffer_row2[i] = ' ';
        lcd_col = 0;
    } else {
		if (lcd_col == COL_LEN)
        	goto end;
    	buffer_row2[lcd_col++] = c;
		lcd.write(c);
    }
end:
  	/* wait until button is released */
  	while (digitalRead(INTRP1))
		;;
	interrupts();
}

void isr_row1() {
	noInterrupts();
	char c = CHAR_MAP_1[keyboard_col];
  	if (lcd_col == COL_LEN)
    	goto end;
    buffer_row2[lcd_col++] = c;
  	lcd.write(c);
end:
    /* wait until button is released */
  	while (digitalRead(INTRP2))
        ;;
  	interrupts();
}

void send_bit(int data) {
  	digitalWrite(DATA, data);
    digitalWrite(CLK, 1);
  	digitalWrite(CLK, 0);
  	digitalWrite(OE, 1);
  	delay(2);
  	digitalWrite(OE, 0);
}

void setup() {
	pinMode(CLK, OUTPUT);
  	pinMode(DATA, OUTPUT);
  	attachInterrupt(digitalPinToInterrupt(INTRP1), isr_row2, RISING);
  	attachInterrupt(digitalPinToInterrupt(INTRP2), isr_row1, RISING);
  	pinMode(13, OUTPUT);
  	lcd.begin(16, 2);
  	lcd.setCursor(0, 1);
}

void loop() {
	keyboard_col = 0;
  	/* Send first bit high */
  	send_bit(1);
  	/* Rest of the bits low */
  	for (keyboard_col = 1; keyboard_col < COL_LEN; keyboard_col++)
    	send_bit(0);
}
