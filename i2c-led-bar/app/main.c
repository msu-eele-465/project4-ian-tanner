#include <msp430fr2355.h>
#include <stdbool.h>
#include <msp430.h> 

unsigned int transition = 32768;
int data_in = 0;
int read_in = 0;
int state = 0;
int led_bar = 0;
int step_0 = 0;
int step_1 = 0;
int step_2 = 0;
int step_3 = 0;
int step_4 = 0;
float trans_scalar = 0;
unsigned char counter_1 = 0;
unsigned char counter_2 = 255;

int main(void)
{

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //status LED
    P1DIR |= BIT4;
    P1OUT &= ~BIT4;

    //LED box
    P1DIR |= BIT1;            //Config P1.2 (A) as output
    P1OUT &= ~BIT1;           //A = 0 to start

    P1DIR |= BIT0;            //Config P1.3 (B) as output
    P1OUT &= ~BIT0;           //B = 0 to start

    P2DIR |= BIT7;            //Config P1.1 (C) as output
    P2OUT &= ~BIT7;           //C = 0 to start

    P2DIR |= BIT6;            //Config P5.4 (D) as output
    P2OUT &= ~BIT6;           //D = 0 to start

    P2DIR |= BIT0;            //Config P4.5 (E) as output
    P2OUT &= ~BIT0;           //E = 0 to start

    P1DIR |= BIT7;            //Config P5.0 (F) as output
    P1OUT &= ~BIT7;           //F = 0 to start

    P1DIR |= BIT6;            //Config P5.2 (G) as output
    P1OUT &= ~BIT6;           //G = 0 to start

    P1DIR |= BIT5;            //Config P5.1 (H) as output
    P1OUT &= ~BIT5;           //H = 0 to start

    //Configure Timers
    TB0CTL |= TBCLR;          //clear timer3 and dividers
    TB0CTL |= TBSSEL__ACLK;   //source = ACLK
    TB0CTL |= MC__UP;         //mode = up
    
    TB0CCR0 = transition;     //TB0CCR0 = 32768

    TB1CTL |= TBCLR;          //clear timer1 and dividers
    TB1CTL |= TBSSEL__ACLK;   //source = ACLK
    TB1CTL |= MC__UP;         //mode = up

    TB1CCR0 = 100;            //TB2CCR0 = 500
    
    TB0CCTL0 &= ~CCIFG;         // Clear CCR0 interrupt flag
    TB0CCTL0 |= CCIE;           // Enable interrupt vector for CCR0

    TB1CCTL0 &= ~CCIFG;         // Clear CCR0 interrupt flag
    TB1CCTL0 |= CCIE;           // Enable interrupt vector for CCR0

    //---------------- Configure B0 for I2C ----------------
    UCB0CTLW0 |= UCSWRST;     //put in SW RST
    UCB0CTLW0 |= UCMODE_3;    //put into I2C mode   
    UCB0CTLW0 &= ~UCMST;      //set as slave
    UCB0CTLW0 &= ~UCTR;       //put into RX mode(read)
    UCB0I2COA0 |= UCOAEN; 
    UCB0I2COA0 |= ECGEN; 
    UCB0I2COA0 = 0x48;        //set slave address LED_bar

    UCB0CTLW1 |= UCASTP_2;    //auto STOP mode
    UCB0TBCNT = 1;            //count = 1 byte

    //---------------- Setup Ports(I2C) ---------------------
    P1SEL1 &= ~BIT3;          //P1.3 = SCL
    P1SEL0 |= BIT3; 

    P1SEL1 &= ~BIT2;          //P1.2 = SDA
    P1SEL0 |= BIT2;

    UCB0IE |= UCRXIE0;        //enable for RX0 IRQ

    __enable_interrupt();       // Enable Global Interrupts

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    UCB0CTLW0 &= ~UCSWRST;    //take B0 out of SW RST

    while(1) {

    }

	return 0;
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void)
{

    switch (state)
    {

        case 0:     // Locked state, jump down

        case 1:     // Unlocking state, jump down

        case 2:     // Unlocked state, no LEDs on
                    //LEDs
                    P1OUT &= ~BIT1;           //A = 0 to start
                    P1OUT &= ~BIT0;           //B = 0 to start
                    P2OUT &= ~BIT7;           //C = 0 to start
                    P2OUT &= ~BIT6;           //D = 0 to start
                    P2OUT &= ~BIT0;           //E = 0 to start
                    P1OUT &= ~BIT7;           //F = 0 to start
                    P1OUT &= ~BIT6;           //G = 0 to start
                    P1OUT &= ~BIT5;           //H = 0 to start
                    break;
        case 3:
                    //LED pattern 0
                    TB0CCR0 = transition;     //TB0CCR0 = base transition period

                    P1OUT |=  BIT1;           //A = 1
                    P1OUT &= ~BIT0;           //B = 0
                    P2OUT |=  BIT7;           //C = 1
                    P2OUT &= ~BIT6;           //D = 0
                    P2OUT |=  BIT0;           //E = 1
                    P1OUT &= ~BIT7;           //F = 0
                    P1OUT |=  BIT6;           //G = 1
                    P1OUT &= ~BIT5;           //H = 0
                    break;

        case 4:
                    //LED pattern 1
                    TB0CCR0 = transition;     //TB0CCR0 = base transition period

                    if (step_0 == 0)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT &= ~BIT5;       //H = 0
                        step_0 = 1;

                    }

                    else if (step_0 == 1)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT |=  BIT5;       //H = 1
                        step_0 = 0;

                    }

                    break;

        case 5:
                    //LED pattern 2
                    trans_scalar = 2;                      //trans_scalar = 2
                    TB0CCR0 = transition/trans_scalar;     //TB0CCR0 = base transition period / trans_scalar

                    //Set A (bit 0 of the counter)
                    if (counter_1 & 0x01)
                    {

                        P1OUT |= BIT1;

                    }

                    else
                    {

                        P1OUT &= ~BIT1;

                    }

                    //Set B (bit 1 of the counter)
                    if (counter_1 & 0x02)
                    {

                         P1OUT |= BIT0;

                    }

                    else
                    {

                        P1OUT &= ~BIT0;

                    }

                    //Set C (bit 2 of the counter)
                    if (counter_1 & 0x04)
                    {

                        P2OUT |= BIT7;

                    }

                    else
                    {

                        P2OUT &= ~BIT7;

                    }

                    //Set D (bit 3 of the counter)
                    if (counter_1 & 0x08)
                    {

                        P2OUT |= BIT6;

                    }

                    else
                    {

                        P2OUT &= ~BIT6;

                    }

                    //Set E (bit 4 of the counter)
                    if (counter_1 & 0x10)
                    {

                        P2OUT |= BIT0;

                    }

                    else
                    {

                        P2OUT &= ~BIT0;

                        }

                    //Set F (bit 5 of the counter)
                    if (counter_1 & 0x20)
                    {

                        P1OUT |= BIT7;

                    }

                    else
                    {

                        P1OUT &= ~BIT7;

                    }

                    //Set G (bit 6 of the counter)
                    if (counter_1 & 0x40)
                    {

                        P1OUT |= BIT6;

                    }

                    else
                    {

                        P1OUT &= ~BIT6;

                    }

                    //Set H (bit 7 of the counter)
                    if (counter_1 & 0x80)
                    {

                        P1OUT |= BIT5;

                    }

                    else
                    {

                        P1OUT &= ~BIT5;

                    }

                    counter_1++;            //increment counter

                    break;

        case 6:
                //LED pattern 3
                trans_scalar = 2;                      //trans_scalar = 2
                TB0CCR0 = transition/trans_scalar;     //TB0CCR0 = base transition period / trans_scalar

                if (step_1 == 0)
                {

                    P1OUT &= ~BIT1;       //A = 0
                    P1OUT &= ~BIT0;       //B = 0
                    P2OUT &= ~BIT7;       //C = 0
                    P2OUT |=  BIT6;       //D = 1
                    P2OUT |=  BIT0;       //E = 1
                    P1OUT &= ~BIT7;       //F = 0
                    P1OUT &= ~BIT6;       //G = 0
                    P1OUT &= ~BIT5;       //H = 0

                    step_1 = 1;           //increment step_1
                }

                else if (step_1 == 1)
                {

                    P1OUT &= ~BIT1;       //A = 0
                    P1OUT &= ~BIT0;       //B = 0
                    P2OUT |=  BIT7;       //C = 1
                    P2OUT &= ~BIT6;       //D = 0
                    P2OUT &= ~BIT0;       //E = 0
                    P1OUT |=  BIT7;       //F = 1
                    P1OUT &= ~BIT6;       //G = 0
                    P1OUT &= ~BIT5;       //H = 0

                    step_1 = 2;           //increment step_1
                }

                else if (step_1 == 2)
                {

                    P1OUT &= ~BIT1;       //A = 0
                    P1OUT |=  BIT0;       //B = 1
                    P2OUT &= ~BIT7;       //C = 0
                    P2OUT &= ~BIT6;       //D = 0
                    P2OUT &= ~BIT0;       //E = 0
                    P1OUT &= ~BIT7;       //F = 0
                    P1OUT |=  BIT6;       //G = 1
                    P1OUT &= ~BIT5;       //H = 0

                    step_1 = 3;           //increment step_1
                }

                else if (step_1 == 3)
                {

                    P1OUT |=  BIT1;       //A = 1
                    P1OUT &= ~BIT0;       //B = 0
                    P2OUT &= ~BIT7;       //C = 0
                    P2OUT &= ~BIT6;       //D = 0
                    P2OUT &= ~BIT0;       //E = 0
                    P1OUT &= ~BIT7;       //F = 0
                    P1OUT &= ~BIT6;       //G = 0
                    P1OUT |=  BIT5;       //H = 1

                    step_1 = 4;           //increment step_1
                }

                else if (step_1 == 4)
                {

                    P1OUT &= ~BIT1;       //A = 0
                    P1OUT |=  BIT0;       //B = 1
                    P2OUT &= ~BIT7;       //C = 0
                    P2OUT &= ~BIT6;       //D = 0
                    P2OUT &= ~BIT0;       //E = 0
                    P1OUT &= ~BIT7;       //F = 0
                    P1OUT |=  BIT6;       //G = 1
                    P1OUT &= ~BIT5;       //H = 0

                    step_1 = 5;           //increment step_1
                }

                else if (step_1 == 5)
                {

                    P1OUT &= ~BIT1;       //A = 0
                    P1OUT &= ~BIT0;       //B = 0
                    P2OUT |=  BIT7;       //C = 1
                    P2OUT &= ~BIT6;       //D = 0
                    P2OUT &= ~BIT0;       //E = 0
                    P1OUT |=  BIT7;       //F = 1
                    P1OUT &= ~BIT6;       //G = 0
                    P1OUT &= ~BIT5;       //H = 0

                    step_1 = 0;           //repeat step_1
                }

                break;

                case 7:
                    //LED pattern 4
                    trans_scalar = 4;                      //trans_scalar = 4
                    TB0CCR0 = transition/trans_scalar;     //TB0CCR0 = base transition period / trans_scalar

                    //Set A (bit 0 of the counter)
                    if (counter_2 & 0x01)
                    {

                        P1OUT |= BIT1;

                    }

                    else
                    {

                        P1OUT &= ~BIT1;

                    }

                    //Set B (bit 1 of the counter)
                    if (counter_2 & 0x02)
                    {

                         P1OUT |= BIT0;

                    }

                    else
                    {

                        P1OUT &= ~BIT0;

                    }

                    //Set C (bit 2 of the counter)
                    if (counter_2 & 0x04)
                    {

                        P2OUT |= BIT7;

                    }

                    else
                    {

                        P2OUT &= ~BIT7;

                    }

                    //Set D (bit 3 of the counter)
                    if (counter_2 & 0x08)
                    {

                        P2OUT |= BIT6;

                    }

                    else
                    {

                        P2OUT &= ~BIT6;

                    }

                    //Set E (bit 4 of the counter)
                    if (counter_2 & 0x10)
                    {

                        P2OUT |= BIT0;

                    }

                    else
                    {

                        P2OUT &= ~BIT0;

                        }

                    //Set F (bit 5 of the counter)
                    if (counter_2 & 0x20)
                    {

                        P1OUT |= BIT7;

                    }

                    else
                    {

                        P1OUT &= ~BIT7;

                    }

                    //Set G (bit 6 of the counter)
                    if (counter_2 & 0x40)
                    {

                        P1OUT |= BIT6;

                    }

                    else
                    {

                        P1OUT &= ~BIT6;

                    }

                    //Set H (bit 7 of the counter)
                    if (counter_2 & 0x80)
                    {

                        P1OUT |= BIT5;

                    }

                    else
                    {

                        P1OUT &= ~BIT5;

                    }

                    counter_2--;            //decrement counter

                    break;

                case 8:
                    //LED pattern 5
                    trans_scalar = 1.5;                    //trans_scalar = 1.5
                    TB0CCR0 = transition*trans_scalar;     //TB0CCR0 = base transition period * trans_scalar

                    if (step_2 == 0)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 1;           //increment step_1
                    }

                    else if (step_2 == 1)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 2;           //increment step_1
                    }

                    else if (step_2 == 2)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7:       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 3;           //increment step_1
                    }

                    else if (step_2 == 3)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 4;           //increment step_1
                    }

                    else if (step_2 == 4)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 5;           //increment step_1
                    }

                    else if (step_2 == 5)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 6;           //repeat step_1
                    }

                    else if (step_2 == 6)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT &= ~BIT5;       //H = 0

                        step_2 = 7;           //increment step_1
                    }

                    else if (step_2 == 7)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT |=  BIT5;       //H = 1

                        step_2 = 0;           //repeat step_1
                    }

                    break;

                case 9:
                    //LED pattern 6
                    trans_scalar = 2;                          //trans_scalar = 0.5
                    TB0CCR0 = transition/trans_scalar;     //TB0CCR0 = base transition period / trans_scalar

                    if (step_3 == 0)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT &= ~BIT5;       //H = 0

                        step_3 = 1;           //increment step_1
                    }

                    else if (step_3 == 1)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 2;           //increment step_1
                    }

                    else if (step_3 == 2)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 3;           //increment step_1
                    }

                    else if (step_3 == 3)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 4;           //increment step_1
                    }

                    else if (step_3 == 4)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 5;           //increment step_1
                    }

                    else if (step_3 == 5)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 6;           //repeat step_1
                    }

                    else if (step_3 == 6)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 7;           //increment step_1
                    }

                    else if (step_3 == 7)
                    {

                        P1OUT &= ~BIT1;       //A = 0
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_3 = 0;           //repeat step_1
                    }

                    break;

                case 10:
                    //LED pattern 7
                    TB0CCR0 = transition;     //TB0CCR0 = base transition period

                    if (step_4 == 0)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT &= ~BIT0;       //B = 0
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 1;           //increment step_1
                    }

                    else if (step_4 == 1)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT &= ~BIT7;       //C = 0
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 2;           //increment step_1
                    }

                    else if (step_4 == 2)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT &= ~BIT6;       //D = 0
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 3;           //increment step_1
                    }

                    else if (step_4 == 3)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT &= ~BIT0;       //E = 0
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 4;           //increment step_1
                    }

                    else if (step_4 == 4)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT &= ~BIT7;       //F = 0
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 5;           //increment step_1
                    }

                    else if (step_4 == 5)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT &= ~BIT6;       //G = 0
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 6;           //repeat step_1
                    }

                    else if (step_4 == 6)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT &= ~BIT5;       //H = 0

                        step_4 = 7;           //increment step_1
                    }

                    else if (step_4 == 7)
                    {

                        P1OUT |=  BIT1;       //A = 1
                        P1OUT |=  BIT0;       //B = 1
                        P2OUT |=  BIT7;       //C = 1
                        P2OUT |=  BIT6;       //D = 1
                        P2OUT |=  BIT0;       //E = 1
                        P1OUT |=  BIT7;       //F = 1
                        P1OUT |=  BIT6;       //G = 1
                        P1OUT |=  BIT5;       //H = 1

                        step_4 = 0;           //repeat step_1
                    }

                    break;

    }

    TB0CCTL0 &= ~CCIFG;          //clear CCR1 flag

}

#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void)
{

    if (data == 0)           //not receiving mode
    {
        P1OUT &= ~BIT6; // Disable GREEN
        P1OUT |= BIT7;  // Enable RED
    }

    else if (data == 1)      //receiving mode
    {
        P1OUT &= ~BIT7;
        P1OUT |= BIT5;
    }

    TB2CCTL0 &= ~CCIFG;          //clear CCR0 flag

}
//---------------- END ISR_TB2_CCR0 ----------------

#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISR_I2C_B0(void){
    read_in = UCB0RXBUF;
    if (read_in == 0x00){
        state = 0;
    }
    else if (read_in == 0x01){
        transition -= 8192;
    }
    else if (read_in == 0x02){
        transition += 8192;
    }
    else if (read_in == 0x03){
        state = 2;
    }
    else if (read_in == 0x04){
        state = 3;
    }
    else if (read_in == 0x05){
        if (state == 4){
            step_0 = 0;
        }
        state = 4;
    }
    else if (read_in == 0x06){
        if (state == 5){
            counter_1 = 0;
        }
        state = 5;
    }
    else if (read_in == 0x07){
        if (state == 6){
            step_1 = 0;
        }
        state = 6;
    }
    else if (read_in == 0x08){
        if (state == 6){
            counter_2 = 255;
        }
        state = 7;
    }
    else if (read_in == 0x09){
        if (state == 8){
            step_2 = 0;
        }
        state = 8;
    }
    else if (read_in == 0x10){
        if (state == 9){
            step_3 = 0;
        }
        state = 9;
    }
    else if (read_in == 0x11){
        if (state == 10){
            step_4 = 0;
        }
        state = 10;
    }
        
}