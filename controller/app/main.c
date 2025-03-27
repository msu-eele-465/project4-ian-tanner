#include <msp430.h> 


#define LCD_ADDRESS 0x01   // Address of the LCD MSP430FR2310
#define TX_BYTES 3    // Number of bytes to transmit

volatile int txIndex = 0;
char txBuffer[TX_BYTES] = {1, 1, 0x42}; // Replace with actual data

void sendI2CData() {
    txIndex = 0; // Reset buffer index

    // Start condition, put master in transmit mode
    UCB0CTLW0 |= UCTR | UCTXSTT;
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//---------------- Configure UCB0 I2C ----------------

	// Configure P1.2 (SDA) and P1.3 (SCL) for I2C
	P1SEL0 |= BIT2 | BIT3;
	P1SEL1 &= ~(BIT2 | BIT3);

	// Put eUSCI_B0 into reset mode
	UCB0CTLW0 = UCSWRST;

	// Set as I2C master, synchronous mode, SMCLK source
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL_3;

	// Manually adjusting baud rate to 100 kHz  (1MHz / 10 = 100 kHz)
	UCB0BRW = 10;

	// Set slave address
	UCB0I2CSA = LCD_ADDRESS;

	// Release reset state
	UCB0CTLW0 &= ~UCSWRST;

	// Enable transmit interrupt
	UCB0IE |= UCTXIE0;
	//---------------- End Configure UCB0 I2C ----------------

	PM5CTL0 &= ~LOCKLPM5; // Unlock GPIO
	__bis_SR_register(GIE); // Enable global interrupts

	sendI2CData();
	while (UCB0CTLW0 & UCTXSTP); // Wait for STOP flag to raise

	while(1);


	return 0;
}

//-------------------------------------------------------------------------------
// Interrupt Service Routines
//-------------------------------------------------------------------------------

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    if (UCB0IV == 0x18) { // TXIFG0 triggered
        if (txIndex < TX_BYTES) {
            UCB0TXBUF = txBuffer[txIndex++]; // Load next byte
        } else {
            UCB0CTLW0 |= UCTXSTP; // Send stop condition
            UCB0IE &= ~UCTXIE0;    // Disable TX interrupt
        }
    }
}
