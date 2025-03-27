#include <msp430.h> 

// Port definitions
#define PXOUT P1OUT
#define PXSEL0 P1SEL0
#define PXSEL1 P1SEL1
#define PXDIR P1DIR

// Pin definitions
#define D4 BIT0
#define D5 BIT1
#define D6 BIT4
#define D7 BIT5

#define E BIT6
#define RS BIT7

// I2C definitions
#define ADDRESS 0x01    // Address for microcontroller
#define RXBYTES 3       // How many bytes we anticipate reading

// LCD Variables
int pattern_index, period_index = 0;

char key[2] = "";

char patterns_array[8][20] = {
    "static", "break", "up counter", "in and out", "down counter", "rotate 1 left", "rotate 7 left", "fill left"
};

char periods_array[6][10] = {
    "0.25s", "0.50s", "0.75s", "1.00s", "1.25s", "1.50s"
};

void lcd_pulse_enable(){
    // Pulses the enable pin so LCD knows to take next nibble.
    PXOUT |= E;         // Enable Enable pin
    PXOUT &= ~E;        // Disable Enable pin
}

void lcd_send_nibble(char nibble){
    // Sends out four bits to the LCD by setting the last four pins (D4 - 7) to the nibble.

    PXOUT &= ~(D4 | D5 | D6 | D7); // Clear the out bits associated with our data lines.

    // For each bit of the nibble, set the associated data line to it.
    if (nibble & BIT0) PXOUT |= D4;
    if (nibble & BIT1) PXOUT |= D5;
    if (nibble & BIT2) PXOUT |= D6;
    if (nibble & BIT3) PXOUT |= D7;

    //PXOUT = (PXOUT &= 0xF0) | (nibble & 0x0F); // First we clear the first four bits, then we set them to the nibble.
    lcd_pulse_enable();  // Pulse enable so LCD reads our input
}

void lcd_send_command(char command){
    // Takes an 8-bit command and sends out the two nibbles sequentially.
    PXOUT &= ~RS; // CLEAR RS to set to command mode
    lcd_send_nibble(command >> 4); // Send upper nibble by bit shifting it to lower nibble
    lcd_send_nibble(command & 0x0F); // Send lower nibble by clearing upper nibble.
}

void lcd_send_data(char data){
    // Takes an 8-bit data and sends out the two nibbles sequentially.
    PXOUT |= RS; // SET RS to set to data mode
    lcd_send_nibble(data >> 4); // Send upper nibble by bit shifting it to lower nibble
    lcd_send_nibble(data & 0x0F); // Send lower nibble by clearing upper nibble.
}

void lcd_print_sentence(char *str){
    // Takes a string and iterates character by character, sending that character to be written out, until \0 is reached.
    while(*str){
        lcd_send_data(*str);
        str++;
    }
}

void lcd_cursor(){
    // Sets cursor properties (visible, blink)
    // When the C key is pressed, the cursor is toggled.
    // When 9 is pressed, the cursor blink is toggled.
    // When NULL is sent, the cursor properties are reset, so both cursor and blink are disabled.
    static char cursor_command = 0b00001100; // Basic command to interface with display (1000) and enable display (0100)
    switch(key[0]){
        case('C'):
            cursor_command ^= BIT1; // Toggle Cursor Enable/Disable bit
            break;
        case('9'):
            cursor_command ^= BIT0; // Toggle Blink Enable/Disable bit
            break;
        case('\0'):
            cursor_command = 0b00001100; // Hard reset cursor info
            break;
        default:
            break;
    }
    lcd_send_command(cursor_command);
}

void lcd_clear(){
    // Clearing the screen is temperamental and requires a good delay, this is pretty arbitrary with a good safety margin.
    lcd_send_command(0x01);   // Clear display
    __delay_cycles(2000);   // Clear display needs some time, I'm aware __delay_cycles generally isn't advised, but it works in this context
}

void lcdInit(){
    // Initializes the LCD to 4 bit mode, 2 lines 5x8 font, with enabled display and cursor,
    // clear the display, then set cursor to proper location.

    PXOUT &= ~RS; // Explicitly set RS to 0 so we are in command mode

    // We need to send the code 3h, 3 times, to properly wake up the LCD screen
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x03);

    lcd_send_nibble(0x02);    // Code 2h sets it to 4-bit mode after waking up

    lcd_send_command(0x28);   // Code 28h sets it to 2 line, 5x8 font.

    lcd_send_command(0x0C);   // Turns display on, turns cursor off, turns blink off.

    lcd_send_command(0x06);   // Increments cursor on each input

    lcd_clear();             // Clear display
}

void lcd_write(){
    /*  Ultimately dictates what will be present on screen after an I2C transmission.
        I2C should come in three bytes, pattern_index, period_index, and key.
        pattern_index -> Integer value corresponding to a pattern in patternArray. Pattern 0 is static, so index 0 is static.
        period_index -> Integer value corresponding to our set periods. Period 0 is 0.25s, the minimum.
        key -> ASCII hex value for the key pressed.

        When the master is locked, or there is no selected pattern, it should send a number >= 8, which will prevent a pattern
        or period from being printed.

        Key is different, if the system is locked then the master should simply send the value 0, which is blank on the LCD.
        Otherwise, it should always send out the ASCII value for the key pressed.
    */
    lcd_clear(); // Clear Display

    if(pattern_index < 8){ // If the I2C code is within pattern_index range
        lcd_send_command(0x80); // Set cursor to line 1 position 1
        lcd_print_sentence(patterns_array[pattern_index]);

        lcd_send_command(0xC0); // Set cursor to line 2 position 1
        lcd_print_sentence("period=");
        lcd_print_sentence(periods_array[period_index]);
    }

    lcd_send_command(0xCF); // Set cursor to line 2 position 16
    lcd_print_sentence(key);
    lcd_cursor(); // Update cursor settings based on key info.

    lcd_send_command(0x80); // Return cursor to line 1 position 1
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//---------------- Configure LCD Ports ----------------

	// Configure Port for digital I/O
	PXSEL0 &= 0x00;
	PXSEL1 &= 0x00;

	PXDIR |= 0XFF;  // SET all bits so Port is OUTPUT mode

	PXOUT &= 0x00;  // CLEAR all bits in output register
	//---------------- End Configure Ports ----------------

    //---------------- Configure UCB0 I2C ----------------

	// Configure P1.2 (SDA) and P1.3 (SCL) for I2C
	P1SEL0 |= BIT2 | BIT3;
	P1SEL1 &= ~(BIT2 | BIT3);

	UCB0CTLW0 = UCSWRST;                 // Put eUSCI in reset
	UCB0CTLW0 |= UCMODE_3 | UCSYNC;      // I2C mode, synchronous mode
	UCB0I2COA0 = ADDRESS | UCOAEN;       // Set slave address and enable
	UCB0CTLW0 &= ~UCSWRST;               // Release eUSCI from reset
	UCB0IE |= UCRXIE0;                   // Enable receive interrupt
	//---------------- End Configure UCB0 I2C ----------------

	PM5CTL0 &= ~LOCKLPM5;       // Clear lock bit
	__bis_SR_register(GIE);     // Enable global interrupts

	lcdInit();

	while(1){

	}

	return 0;
}

//-------------------------------------------------------------------------------
// Interrupt Service Routines
//-------------------------------------------------------------------------------

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    /* The microcontroller expects three bytes to be transmitted from the master.
     * These bytes are sequentially added to the pattern_index, period_index, and key values.
     *
     * These values are then processed by lcd_write(), where more information can be found about their handling.
     */
    static int byte_count = 0;
    if(UCB0IV == 0x16){  // RXIFG0 Flag, RX buffer is full and can be processed
        switch(byte_count){
            case 0:
                pattern_index = UCB0RXBUF;
                break;
            case 1:
                period_index = UCB0RXBUF;
                break;
            case 2:
                key[0] = UCB0RXBUF;
                break;
            default:
                break;
        }
        byte_count++;
        if(byte_count >= 3){
            byte_count = 0;
            lcd_write();
        }
    }
}
