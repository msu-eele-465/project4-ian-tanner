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

#define RS BIT6
#define E BIT7

// I2C definitions
#define ADDRESS 0x01    // Address for microcontroller
#define RXBYTES 3       // How many bytes we anticipate reading


/**
 * main.c
 */

int patternIndex, periodIndex = 0;

char patterns[8][20] = {
    "static", "break", "up counter", "in and out", "down counter", "rotate 1 left", "rotate 7 left", "fill left"
};

char *key = "A";
char *period = "0.25s";

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
    while(*str){
        lcdSendData(*str);
        str++;
    }
}

void lcdClear(){
    // Clearing the screen is temperamental and requires a good delay, this is pretty arbitrary with a good safety margin.
    lcdSendCommand(0x01);   // Clear display
    __delay_cycles(2000);   // Clear display needs some time
}

void lcdInit(){
    // Initializes the LCD to 4 bit mode, 2 lines 5x8 font, with enabled display and cursor,
    // clear the display, then set cursor to proper location.

    PXOUT &= ~RS; // Explicitly set RS to 0 so we are in command mode

    // We need to send the code 3h, 3 times, with some delay to actually wake it up.
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
    lcdClear(); // Clear Display

    lcdSendCommand(0x80); // Set cursor to line 1 position 1
    lcdPrintSentence(patterns[patternIndex]);
    lcdSendCommand(0xC0); // Set cursor to line 2 position 1
    lcdPrintSentence("period=");
    lcdPrintSentence(period);
    lcdSendCommand(0xCF); // Set cursor to line 2 position 16
    lcdPrintSentence(key);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//---------------- Configure LCD Ports ----------------
	/* This code was initially written for our normal microcontroller just for ease of programming. "Port" is ambiguous,
	as I use macros and #define statements so that I can easily change ports when I move this to the little microcontroller.
	*/

	// We want to use pins 0 - 3 for D4 - 7 and pins 4 and 5 for RS and E, respectively

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

	lcdInit();

	while(1){

	    /*lcdClear(); // Clear Display

	    lcdSendCommand(0x80); // Set cursor to line 1 position 1
	    lcdPrintSentence(patterns[patternIndex]);
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
    int byteCount = 0;
    if(UCB0IV = 0x16){  // RXIFG0 Flag, RX buffer is full and can be processed
        switch(byteCount){
            case 0:
                patternIndex = atoi(UCB0RXBUF);
                break;
            case 1:
                periodIndex = atoi(UCB0RXBUF);
                break;
            case 2:
                *key = UCB0RXBUF;
                break;
            default:
                break;
        }
        byteCount++;
        if(byteCount >= 3){
            byteCount = 0;
        }
        lcdWrite();
    }
}
