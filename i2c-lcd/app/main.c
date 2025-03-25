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


/**
 * main.c
 */

int patternIndex, periodIndex = 0;

char key[2] = "";

char patternsArray[8][20] = {
    "static", "break", "up counter", "in and out", "down counter", "rotate 1 left", "rotate 7 left", "fill left"
};

char periodsArray[6][10] = {
    "0.25s", "0.50s", "0.75s", "1.00s", "1.25s", "1.50s"
};

void lcdPulseEnable(){
    // Pulses the enable pin so LCD knows to take next nibble.
    PXOUT |= E;         // Enable Enable pin
    PXOUT &= ~E;        // Disable Enable pin
}

void lcdSendNibble(char nibble){
    // Sends out four bits to the LCD by setting the last four pins (D4 - 7) to the nibble.

    PXOUT &= ~(D4 | D5 | D6 | D7); // Clear the out bits associated with our data lines.

    // For each bit of the nibble, set the associated data line to it.
    if (nibble & BIT0) PXOUT |= D4;
    if (nibble & BIT1) PXOUT |= D5;
    if (nibble & BIT2) PXOUT |= D6;
    if (nibble & BIT3) PXOUT |= D7;

    //PXOUT = (PXOUT &= 0xF0) | (nibble & 0x0F); // First we clear the first four bits, then we set them to the nibble.
    lcdPulseEnable();  // Pulse enable so LCD reads our input
}

void lcdSendCommand(char command){
    // Takes an 8-bit command and sends out the two nibbles sequentially.
    PXOUT &= ~RS; // CLEAR RS to set to command mode
    lcdSendNibble(command >> 4); // Send upper nibble by bit shifting it to lower nibble
    lcdSendNibble(command & 0x0F); // Send lower nibble by clearing upper nibble.
}

void lcdSendData(char data){
    // Takes an 8-bit data and sends out the two nibbles sequentially.
    PXOUT |= RS; // SET RS to set to data mode
    lcdSendNibble(data >> 4); // Send upper nibble by bit shifting it to lower nibble
    lcdSendNibble(data & 0x0F); // Send lower nibble by clearing upper nibble.
}

void lcdPrintSentence(char *str){
    // Takes a string and iterates character by character, sending that character to be written out, until \0 is reached.
    while(*str){
        lcdSendData(*str);
        str++;
    }
}

void lcdCursor(){
    // Sets cursor properties (visible, blink)
    static char cursorCommand = 0b00001100; // Basic command to interface with display (1000) and enable display (0100)
    switch(key[0]){
        case('C'):
            cursorCommand ^= BIT1; // Toggle Cursor Enable/Disable bit
            break;
        case('9'):
            cursorCommand ^= BIT0; // Toggle Blink Enable/Disable bit
            break;
        case('\0'):
            cursorCommand = 0b00001100; // Hard reset cursor info
            break;
        default:
            break;
    }
    lcdSendCommand(cursorCommand);

}

void lcdClear(){
    // Clearing the screen is temperamental and requires a good delay, this is pretty arbitrary with a good safety margin.
    lcdSendCommand(0x01);   // Clear display
    __delay_cycles(2000);   // Clear display needs some time, I'm aware __delay_cycles generally isn't advised
}

void lcdInit(){
    // Initializes the LCD to 4 bit mode, 2 lines 5x8 font, with enabled display and cursor,
    // clear the display, then set cursor to proper location.

    PXOUT &= ~RS; // Explicitly set RS to 0 so we are in command mode

    // We need to send the code 3h, 3 times, to properly wake up the LCD screen
    lcdSendNibble(0x03);
    lcdSendNibble(0x03);
    lcdSendNibble(0x03);

    lcdSendNibble(0x02);    // Code 2h sets it to 4-bit mode after waking up

    lcdSendCommand(0x28);   // Code 28h sets it to 2 line, 5x8 font.

    lcdSendCommand(0x0C);   // Turns display on, turns cursor off, turns blink off.

    lcdSendCommand(0x06);   // Increments cursor on each input

    lcdClear();             // Clear display
}

void lcdWrite(){
    /*  Ultimately dictates what will be present on screen after an I2C transmission.
        I2C should come in three bytes, patternIndex, periodIndex, and key.
        patternIndex -> Integer value corresponding to a pattern in patternArray. Pattern 0 is static, so index 0 is static.
        periodIndex -> Integer value corresponding to our set periods. Period 0 is 0.25s, the minimum.
        key -> ASCII hex value for the key pressed.

        When the master is locked, or there is no selected pattern, it should send the number 9, which will prevent a pattern
        or period from being printed.

        Key is different, if the system is locked then the master should simply send the value 0, which is blank on the LCD.
        Otherwise, it should always send out the ASCII value for the key pressed.
    */
    lcdClear(); // Clear Display

    if(patternIndex < 8){ // If the I2C code is within patternIndex range
        lcdSendCommand(0x80); // Set cursor to line 1 position 1
        lcdPrintSentence(patternsArray[patternIndex]);

        lcdSendCommand(0xC0); // Set cursor to line 2 position 1
        lcdPrintSentence("period=");
        lcdPrintSentence(periodsArray[periodIndex]);
    }

    lcdSendCommand(0xCF); // Set cursor to line 2 position 16
    lcdPrintSentence(key);
    lcdCursor(); // Update cursor settings based on key info.

    lcdSendCommand(0x80); // Return cursor to line 1 position 1
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
	__bis_SR_register(GIE);  // Enable global interrupts

	lcdInit();

	while(1){

	    /*lcdClear(); // Clear Display

	    lcdSendCommand(0x80); // Set cursor to line 1 position 1
	    lcdPrintSentence(patternsArray[patternIndex]);
	    lcdSendCommand(0xC0); // Set cursor to line 2 position 1
	    lcdPrintSentence("period=");
	    lcdPrintSentence(period);
	    lcdSendCommand(0xCF); // Set cursor to line 2 position 16
	    lcdPrintSentence(key);

	    if(patternIndex >= 7){
	        patternIndex = 0;
	    }else{
	        patternIndex++;
	    }
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);
	    __delay_cycles(60000);*/
	}

	return 0;
}

//-------------------------------------------------------------------------------
// Interrupt Service Routines
//-------------------------------------------------------------------------------

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    /* The microcontroller expects three bytes to be transmitted from the master.
     * These bytes are sequentially added to the patternIndex, periodIndex, and key values.
     * These values are then processed by lcdWrite(), where more information can be found about their handling.
     */
    static int byteCount = 0;
    if(UCB0IV == 0x16){  // RXIFG0 Flag, RX buffer is full and can be processed
        switch(byteCount){
            case 0:
                patternIndex = UCB0RXBUF;
                break;
            case 1:
                periodIndex = UCB0RXBUF;
                break;
            case 2:
                key[0] = UCB0RXBUF;
                break;
            default:
                break;
        }
        byteCount++;
        if(byteCount >= 3){
            byteCount = 0;
            lcdWrite();
        }
    }
}
