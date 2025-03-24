#include <msp430fr2355.h>
#include <stdbool.h>
#include <msp430.h> 


/**
 * main.c
 */


// 2D Array, each array is a row, each item is a column.

char keyPad[][4] = {{'1', '2', '3', 'A'},  // Top Row
                    {'4', '5', '6', 'B'},
                    {'7', '8', '9', 'C'},
                    {'*', '0', '#', 'D'}}; // Bottom Row
/*                    ^              ^
 *                    |              |
 *                    Left Column    Right Column
 */

int column, row = 0;

char keyPressed = '\0';

char passCode[] = "2659";
char inputCode[] = "0000";

int miliSecondsSurpassed = 0;

int index = 0;  // Which index of the above inputCode array we're in
int state = 0;  // State 0: Locked, State 1: Unlocking, State 2: Unlocked
int tx_state = 0;
unsigned int transition = 32768;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    //---------------- Configure TB0 ----------------
    TB0CTL |= TBCLR;            // Clear TB0 timer and dividers
    TB0CTL |= TBSSEL__SMCLK;    // Select SMCLK as clock source
    TB0CTL |= MC__UP;            // Choose UP counting

    TB0CCR0 = 1000;             // TTB0CCR0 = 1000, since 1/MHz * 1000 = 1 ms
    TB0CCTL0 &= ~CCIFG;         // Clear CCR0 interrupt flag
    TB0CCTL0 |= CCIE;           // Enable interrupt vector for CCR0

    //---------------- End Configure TB0 ----------------

    //---------------- Configure P3 ----------------
    // Configure P3 for digital I/O
    P3SEL0 &= 0x00;
    P3SEL1 &= 0x00;

    P3DIR &= 0x0F;  // CLEARING bits 7 - 4, that way they are set to INPUT mode
    P3DIR |= 0X0F;  // SETTING bits 0 - 3, that way they are set to OUTPUT mode

    P3REN |= 0xF0;  // ENABLING the resistors for bits 7 - 4
    P3OUT &= 0x00;  // CLEARING output register. This both clears our outputs on bits 0 - 3, and sets pull-down resistors
                    // for bits 7 - 4
    //---------------- End Configure P3 ----------------

    //---------------- Configure LEDs ----------------
    //Heartbeat LEDs
    P1DIR |= BIT0;            //Config P1.0 (LED1) as output
    P1OUT |= BIT0;            //LED1 = 1 to start

    P6DIR |= BIT6;            //Config P6.6 (LED2) as output
    P6OUT &= ~BIT6;           //LED2 = 0 to start

    //RGB LEDs
    P1DIR |= BIT7;            //Config P1.0 (RED) as output
    P1OUT &= ~BIT7;           //RED = 0 to start

    P1DIR |= BIT6;            //Config P6.6 (BLUE) as output
    P1OUT &= ~BIT6;           //BLUE = 0 to start

    P1DIR |= BIT5;            //Config P1.0 (GREEN) as output
    P1OUT &= ~BIT5;           //GREEN = 0 to start

    //---------------- Configure LED Timers ----------------
    //Setup Timers
    TB1CTL |= TBCLR;          //clear timer1 and dividers
    TB1CTL |= TBSSEL__ACLK;   //source = ACLK
    TB1CTL |= MC__UP;         //mode = up

    TB2CTL |= TBCLR;          //clear timer2 and dividers
    TB2CTL |= TBSSEL__ACLK;   //source = ACLK
    TB2CTL |= MC__UP;         //mode = up

    TB1CCR0 = 32768;          //TB1CCR0 = 32768
    TB2CCR0 = 500;            //TB2CCR0 = 500
    TB2CCR1 = 27;             //TB2CCR1 = 27

    TB1CCTL0 |= CCIE;         //enable TB1 CCR0 Overflow IRQ
    TB1CCTL0 &= ~CCIFG;       //clear CCR0 flag

    TB2CCTL0 |= CCIE;         //enable TB2 CCR0 Overflow IRQ
    TB2CCTL0 &= ~CCIFG;       //clear CCR0 flag

    TB2CCTL1 |= CCIE;         //enable TB2 CCR1 Overflow IRQ
    TB2CCTL1 &= ~CCIFG;       //clear CCR1 flag

    //---------------- Configure B0 for I2C ----------------
    UCB0CTLW0 |= UCSWRST;     //put in SW RST

    UCB0CTLW0 |= UCSSEL_3;    //choose SMCLK
    UCB0BRW = 10;             //set prescalar to 10

    UCB0CTLW0 |= UCMODE_3;    //put into I2C mode
    UCB0CTLW0 |= UCMST;       //set as master
    UCB0CTLW0 |= UCTR;        //put into TX mode(write)
    UCB0I2CSA = 0x48;         //set slave address LED_bar

    UCB0CTLW1 |= UCASTP_2;    //auto STOP mode
    UCB0TBCNT = 1;            //count = 1 byte

    //---------------- Setup Ports(I2C) ---------------------
    P1SEL1 &= ~BIT3;          //P1.3 = SCL
    P1SEL0 |= BIT3; 

    P1SEL1 &= ~BIT2;          //P1.2 = SDA
    P1SEL0 |= BIT2;

    UCB0IE |= UCTXIE0;        //enable for TX0 IRQ

    __enable_interrupt();     // Enable Global Interrupts
    PM5CTL0 &= ~LOCKLPM5;     // Clear lock bit

    UCB0CTLW0 &= ~UCSWRST;    //take B0 out of SW RST

    while(1) {}

	return 0;
}

//-------------------------------------------------------------------------------
// Interrupt Service Routines
//-------------------------------------------------------------------------------

//---------------- START ISR_TB0_SwitchColumn ----------------
//-- TB0 CCR0 interrupt, read row data from column, shift roll read column right
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_SwitchColumn(void)
{

    if(state == 1){ // If in unlocking state
        if(miliSecondsSurpassed >= 5000){
            state = 0; // Set to lock state
            index = 0; // Reset position on inputCode
            miliSecondsSurpassed = 0; // Reset timeout counter
        }else{
            miliSecondsSurpassed++;
        }
    }

    switch (column) {
        case 0:
            P3OUT = 0b00001000; //Enable reading far left column
            break;
        case 1:
            P3OUT = 0b00000100; // Enable reading center left column
            break;
        case 2:
            P3OUT = 0b00000010; // Enable reading center right column
            break;
        default: // Case 3
            P3OUT = 0b00000001; // Enable reading far right column
    }

    if(P3IN > 15){  // If a button is being pressed

        if(state == 0){ // If we're in the locked state, go to unlocking state.
            state = 1;
        }

        if(P3IN & BIT4){    // If bit 4 is receiving input, we're at row 3, so on and so forth
            row = 3;
        }else if(P3IN & BIT5){
            row = 2;
        }else if(P3IN & BIT6){
            row = 1;
        }else if(P3IN & BIT7){
            row = 0;
        }

        keyPressed = keyPad[row][column];

        switch(state){

            case 1: // If unlocking, we populate our input code with each pressed key
                inputCode[index] = keyPressed; // Set the input code at index to what is pressed.

                if(index >= 3){ // If we've entered all four digits of input code:
                    index = 0;
                    state = 2; // Initially set state to free
                    miliSecondsSurpassed = 0; // Stop lockout counter
                    int i;
                    for(i = 0; i < 4; i++){ // Iterate through the passCode and inputCode
                        if(inputCode[i] != passCode[i]){ // If an element in passCode and inputCode doesn't match
                            state = 0;                   // Set state back to locked.
                            break;
                        }
                    }
                }else{
                    index++; // Shift to next index of input code
                }

                break;

            default:     // If unlocked, we check the individual key press.
                switch(keyPressed){
                    case('D'):
                        state = 0; // Enter locked mode
                        tx_state = 0;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('A'):
                        tx_state = 1;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('B'):
                        tx_state = 2;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('0'):      // Pattern 0
                        tx_state = 3;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('1'):      // Pattern 1
                        tx_state = 4;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('2'):      // Pattern 2
                        tx_state = 5;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('3'):      // Pattern 3
                        tx_state = 6;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('4'):      // Pattern 4
                        tx_state = 7;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('5'):      // Pattern 5
                        tx_state = 8;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('6'):      // Pattern 6
                        tx_state = 9;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    case('7'):      // Pattern 7
                        tx_state = 10;
                        UCB0CTLW0 |= UCTXSTT;      //start message (START)
                        break;
                    default:
                        break;
                }
                break;
        }

        while(P3IN > 15){} // Wait until button is released

    }

    if(P3IN < 16){ // Checks if pins 7 - 4 are on, that means a button is being held down; don't shift columns
        if (++column >= 4) {column = 0;} // Add one to column, if it's 4 reset back to 0.
    }
    TB0CCTL0 &= ~TBIFG;
}
//---------------- End ISR_TB0_SwitchColumn ----------------

//---------------- START ISR_TB1_Heartbeat ----------------
// Heartbeat function
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_Heartbeat(void)
{
    P1OUT ^= BIT0;               //Toggle P1.0(LED1)
    P6OUT ^= BIT6;               //Toggle P6.6(LED2)
    TB1CCTL0 &= ~CCIFG;          //clear CCR0 flag
}
//---------------- END ISR_TB1_Heartbeat ----------------

//---------------- START ISR_TB2_CCR0 ----------------
// Controls RGB Light
#pragma vector = TIMER2_B0_VECTOR
__interrupt void ISR_TB2_CCR0(void)
{

    if (state == 0)           //locked mode
    {
        P1OUT &= ~BIT5; // Disable BLUE
        P1OUT &= ~BIT6; // Disable GREEN
        P1OUT |= BIT7;  // Enable RED
    }

    else if (state == 1)      //unlocking mode
    {
        P1OUT |= BIT7;
        P1OUT |= BIT5;
    }

    else if (state == 2)      //unlocked mode
    {
        P1OUT |= BIT6;
        P1OUT |= BIT5;
    }
    TB2CCTL0 &= ~CCIFG;          //clear CCR0 flag
}

#pragma vector = TIMER2_B1_VECTOR
__interrupt void ISR_TB2_CCR1(void){

    if (state == 0)           //locked mode
    {
        P1OUT |= BIT7;
    }

    else if (state == 1)      //unlocking mode
    {
        TB2CCR1 = 27;           //TB2CCR1 = 27
        P1OUT &= ~BIT5;
    }

    else if (state == 2)      //unlocked
    {
        TB2CCR1 = 200;          //TB2CCR1 = 200
        P1OUT &= ~BIT5;
    }
    TB2CCTL1 &= ~CCIFG;         //clear CCR1 flag
}

#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISR_I2C_B0(void){
    switch (tx_state)
    {
        case 0:
                UCB0TXBUF = 0x01;
                break;
        case 1:
                UCB0TXBUF = 0x02;
                break;
        case 2:
                UCB0TXBUF = 0x03;
                break;
        case 3:
                UCB0TXBUF = 0x04;
                break;
        case 4:
                UCB0TXBUF = 0x05;
                break;
        case 5:
                UCB0TXBUF = 0x06;
                break;
        case 6:
                UCB0TXBUF = 0x07;
                break;
        case 7:
                UCB0TXBUF = 0x08;
                break;
        case 8:
                UCB0TXBUF = 0x09;
                break;
        case 9:
                UCB0TXBUF = 0x10;
                break;
        case 10:
                UCB0TXBUF = 0x11;
                break;
    }
}