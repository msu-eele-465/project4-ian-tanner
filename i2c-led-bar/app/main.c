#include <msp430fr2310.h>
#include <stdbool.h>
#include <msp430.h> 


//instantiate variables
enum state {pattern_off, pattern_0, pattern_1, pattern_2, pattern_3, pattern_4, pattern_5, pattern_6, pattern_7};
enum state led_state = pattern_off;
int data_in = 0;
int read_in = 0;
int step_pattern_1 = 0;
int step_pattern_3 = 0;
int step_pattern_5 = 0;
int step_pattern_6 = 0;
int step_pattern_7 = 0;
unsigned int counter_pattern_2 = 0;
unsigned int counter_pattern_4 = 255;
unsigned int transition = 32768;
float trans_scalar = 0;

int main(void)
{

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //Status LED
    P1DIR |= BIT4;
    P1OUT &= ~BIT4;

    //LED box
    P1DIR |= BIT1;           
    P1OUT &= ~BIT1;           

    P1DIR |= BIT0;           
    P1OUT &= ~BIT0;           

    P2DIR |= BIT7;            
    P2OUT &= ~BIT7;           

    P2DIR |= BIT6;           
    P2OUT &= ~BIT6;           

    P2DIR |= BIT0;          
    P2OUT &= ~BIT0;          

    P1DIR |= BIT7;            
    P1OUT &= ~BIT7;           

    P1DIR |= BIT6;            
    P1OUT &= ~BIT6;           

    P1DIR |= BIT5;            
    P1OUT &= ~BIT5;           

    //Configure Timers
    TB0CTL |= TBCLR;          
    TB0CTL |= TBSSEL__ACLK;   
    TB0CTL |= MC__UP;         
    TB0CCR0 = transition;     

    TB1CTL |= TBCLR;          
    TB1CTL |= TBSSEL__ACLK;   
    TB1CTL |= MC__UP;         
    TB1CCR0 = 100;            
    
    //Enable timer interrupts
    TB0CCTL0 &= ~CCIFG;        
    TB0CCTL0 |= CCIE;           

    TB1CCTL0 &= ~CCIFG;         
    TB1CCTL0 |= CCIE;           

    // Configure B0 for I2C 
    UCB0CTLW0 |= UCSWRST;                
	UCB0CTLW0 = UCSWRST;                 
	UCB0CTLW0 |= UCMODE_3 | UCSYNC;      
	UCB0I2COA0 = 0x48 | UCOAEN;       
	UCB0CTLW0 &= ~UCSWRST;               
	UCB0IE |= UCRXIE0;                   

    //Configure P1.2 (SDA) and P1.3 (SCL) for I2C
	P1SEL0 |= BIT2 | BIT3;
	P1SEL1 &= ~(BIT2 | BIT3);       

    //Enable global interrupts
    __bis_SR_register(GIE);       

    //Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;    

    //Loop forever
    while(1)
    {

    }

	return 0;
}

//Timer B0 interrupt
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void)
{

    switch (led_state)
    {     

        case pattern_off:    

                    P1OUT &= ~BIT1;          
                    P1OUT &= ~BIT0;           
                    P2OUT &= ~BIT7;           
                    P2OUT &= ~BIT6;          
                    P2OUT &= ~BIT0;          
                    P1OUT &= ~BIT7;           
                    P1OUT &= ~BIT6;          
                    P1OUT &= ~BIT5;          
                    break;

        case pattern_0:
                    
                    TB0CCR0 = transition;     

                    P1OUT |=  BIT1;           
                    P1OUT &= ~BIT0;           
                    P2OUT |=  BIT7;           
                    P2OUT &= ~BIT6;           
                    P2OUT |=  BIT0;           
                    P1OUT &= ~BIT7;          
                    P1OUT |=  BIT6;           
                    P1OUT &= ~BIT5;           
                    break;

        case pattern_1:
                   
                    TB0CCR0 = transition;    

                    if (step_pattern_1 == 0)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT |=  BIT6;      
                        P1OUT &= ~BIT5;       
                        step_pattern_1++;

                    }

                    else if (step_pattern_1 == 1)
                    {

                        P1OUT &= ~BIT1;      
                        P1OUT |=  BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT |=  BIT5;       
                        step_pattern_1--;

                    }

                    break;

        case pattern_2:

                    trans_scalar = 2;                     
                    TB0CCR0 = transition/trans_scalar;     

                    
                    if (counter_pattern_2 & 0x01)
                    {

                        P1OUT |= BIT1;

                    }

                    else
                    {

                        P1OUT &= ~BIT1;

                    }

                    
                    if (counter_pattern_2 & 0x02)
                    {

                         P1OUT |= BIT0;

                    }

                    else
                    {

                        P1OUT &= ~BIT0;

                    }

                   
                    if (counter_pattern_2 & 0x04)
                    {

                        P2OUT |= BIT7;

                    }

                    else
                    {

                        P2OUT &= ~BIT7;

                    }

                    
                    if (counter_pattern_2 & 0x08)
                    {

                        P2OUT |= BIT6;

                    }

                    else
                    {

                        P2OUT &= ~BIT6;

                    }

                    
                    if (counter_pattern_2 & 0x10)
                    {

                        P2OUT |= BIT0;

                    }

                    else
                    {

                        P2OUT &= ~BIT0;

                        }

                    
                    if (counter_pattern_2 & 0x20)
                    {

                        P1OUT |= BIT7;

                    }

                    else
                    {

                        P1OUT &= ~BIT7;

                    }

                    
                    if (counter_pattern_2 & 0x40)
                    {

                        P1OUT |= BIT6;

                    }

                    else
                    {

                        P1OUT &= ~BIT6;

                    }

                    
                    if (counter_pattern_2 & 0x80)
                    {

                        P1OUT |= BIT5;

                    }

                    else
                    {

                        P1OUT &= ~BIT5;

                    }

                    counter_pattern_2++;            

                    break;

        case pattern_3:
        
                trans_scalar = 2;                      
                TB0CCR0 = transition/trans_scalar;     

                if (step_pattern_3 == 0)
                {

                    P1OUT &= ~BIT1;       
                    P1OUT &= ~BIT0;       
                    P2OUT &= ~BIT7;       
                    P2OUT |=  BIT6;       
                    P2OUT |=  BIT0;       
                    P1OUT &= ~BIT7;       
                    P1OUT &= ~BIT6;       
                    P1OUT &= ~BIT5;      

                    step_pattern_3++;           
                }

                else if (step_pattern_3 == 1)
                {

                    P1OUT &= ~BIT1;       
                    P1OUT &= ~BIT0;       
                    P2OUT |=  BIT7;       
                    P2OUT &= ~BIT6;       
                    P2OUT &= ~BIT0;       
                    P1OUT |=  BIT7;       
                    P1OUT &= ~BIT6;       
                    P1OUT &= ~BIT5;       

                    step_pattern_3++;           
                }

                else if (step_pattern_3 == 2)
                {

                    P1OUT &= ~BIT1;       
                    P1OUT |=  BIT0;       
                    P2OUT &= ~BIT7;       
                    P2OUT &= ~BIT6;       
                    P2OUT &= ~BIT0;       
                    P1OUT &= ~BIT7;       
                    P1OUT |=  BIT6;       
                    P1OUT &= ~BIT5;       

                    step_pattern_3++;           
                }

                else if (step_pattern_3 == 3)
                {

                    P1OUT |=  BIT1;       
                    P1OUT &= ~BIT0;       
                    P2OUT &= ~BIT7;       
                    P2OUT &= ~BIT6;       
                    P2OUT &= ~BIT0;       
                    P1OUT &= ~BIT7;       
                    P1OUT &= ~BIT6;       
                    P1OUT |=  BIT5;       

                    step_pattern_3++;           
                }

                else if (step_pattern_3 == 4)
                {

                    P1OUT &= ~BIT1;       
                    P1OUT |=  BIT0;       
                    P2OUT &= ~BIT7;       
                    P2OUT &= ~BIT6;       
                    P2OUT &= ~BIT0;       
                    P1OUT &= ~BIT7;      
                    P1OUT |=  BIT6;       
                    P1OUT &= ~BIT5;       

                    step_pattern_3++;
                }

                else if (step_pattern_3 == 5)
                {

                    P1OUT &= ~BIT1;       
                    P1OUT &= ~BIT0;       
                    P2OUT |=  BIT7;       
                    P2OUT &= ~BIT6;       
                    P2OUT &= ~BIT0;       
                    P1OUT |=  BIT7;       
                    P1OUT &= ~BIT6;       
                    P1OUT &= ~BIT5;       

                    step_pattern_3 = 0;
                }

                break;

                case pattern_4:
                    
                    trans_scalar = 4;                      
                    TB0CCR0 = transition/trans_scalar;     

                    
                    if (counter_pattern_4 & 0x01)
                    {

                        P1OUT |= BIT1;

                    }

                    else
                    {

                        P1OUT &= ~BIT1;

                    }

                    
                    if (counter_pattern_4 & 0x02)
                    {

                         P1OUT |= BIT0;

                    }

                    else
                    {

                        P1OUT &= ~BIT0;

                    }

                    
                    if (counter_pattern_4 & 0x04)
                    {

                        P2OUT |= BIT7;

                    }

                    else
                    {

                        P2OUT &= ~BIT7;

                    }

                    
                    if (counter_pattern_4 & 0x08)
                    {

                        P2OUT |= BIT6;

                    }

                    else
                    {

                        P2OUT &= ~BIT6;

                    }

                    
                    if (counter_pattern_4 & 0x10)
                    {

                        P2OUT |= BIT0;

                    }

                    else
                    {

                        P2OUT &= ~BIT0;

                        }

                    
                    if (counter_pattern_4 & 0x20)
                    {

                        P1OUT |= BIT7;

                    }

                    else
                    {

                        P1OUT &= ~BIT7;

                    }

                    
                    if (counter_pattern_4 & 0x40)
                    {

                        P1OUT |= BIT6;

                    }

                    else
                    {

                        P1OUT &= ~BIT6;

                    }

                    
                    if (counter_pattern_4 & 0x80)
                    {

                        P1OUT |= BIT5;

                    }

                    else
                    {

                        P1OUT &= ~BIT5;

                    }

                    counter_pattern_4--;            

                    break;

                case pattern_5:
                    
                    trans_scalar = 1.5;                    
                    TB0CCR0 = transition*trans_scalar;     

                    if (step_pattern_5 == 0)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;      
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;          
                    }

                    else if (step_pattern_5 == 1)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;
                    }

                    else if (step_pattern_5 == 2)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;      

                        step_pattern_5++;
                    }

                    else if (step_pattern_5 == 3)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;           
                    }

                    else if (step_pattern_5 == 4)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;
                    }

                    else if (step_pattern_5 == 5)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;
                    }

                    else if (step_pattern_5 == 6)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;      
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;      
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_5++;
                    }

                    else if (step_pattern_5 == 7)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_5 = 0;          
                    }

                    break;

                case pattern_6:
                
                    trans_scalar = 2;                          
                    TB0CCR0 = transition/trans_scalar;     

                    if (step_pattern_6 == 0)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 1)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;      
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT &= ~BIT6;      
                        P1OUT |=  BIT5;      

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 2)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 3)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 4)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 5)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 6)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_6++;           
                    }

                    else if (step_pattern_6 == 7)
                    {

                        P1OUT &= ~BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;      

                        step_pattern_6 = 0;           
                    }

                    break;

                case pattern_7:
                    
                    TB0CCR0 = transition;     

                    if (step_pattern_7 == 0)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT &= ~BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;      
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;      

                        step_pattern_7++;           
                    }

                    else if (step_pattern_7 == 1)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT &= ~BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;      
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;          
                    }

                    else if (step_pattern_7 == 2)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT &= ~BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;           
                    }

                    else if (step_pattern_7 == 3)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT &= ~BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;           
                    }

                    else if (step_pattern_7 == 4)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT &= ~BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;           
                    }

                    else if (step_pattern_7 == 5)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT &= ~BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;          
                    }

                    else if (step_pattern_7 == 6)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;       
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT &= ~BIT5;       

                        step_pattern_7++;           
                    }

                    else if (step_pattern_7 == 7)
                    {

                        P1OUT |=  BIT1;       
                        P1OUT |=  BIT0;      
                        P2OUT |=  BIT7;       
                        P2OUT |=  BIT6;       
                        P2OUT |=  BIT0;       
                        P1OUT |=  BIT7;       
                        P1OUT |=  BIT6;       
                        P1OUT |=  BIT5;       

                        step_pattern_7 = 0;           
                    }

                    break;

    }
    //clear CCR1 flag
    TB0CCTL0 &= ~CCIFG;          

}

//Timer B1 interrupt
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void)
{

    if (data_in == 0)           
    {

        P1OUT |= BIT7;  

    }

    else if (data_in == 1)      
    {

        P1OUT &= ~BIT7;
        
    }

    TB1CCTL0 &= ~CCIFG;          //clear CCR0 flag

}

//RX interrupt
#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISR_I2C_B0(void)
{
    read_in = UCB0RXBUF;

    switch (read_in)
    {

        case 0x01:

            led_state = pattern_off;

            break;

        case 0x02:

            transition -= 8192;

            break;

        case 0x03:

            transition += 8192;

            break;

        case 0x04:

            led_state = pattern_0;

            break;    

        case 0x05:

            if (led_state == pattern_1)
            {
                
                step_pattern_1 = 0;

            }

            led_state = pattern_1;

            break;    

        case 0x06:

            if (led_state == pattern_2)
            {

                counter_pattern_2 = 0;

            }

            led_state = pattern_2;

            break;

        case 0x07:

            if (led_state == pattern_3)
            {

                step_pattern_3 = 0;

            }

            led_state = pattern_3;

            break;

        case 0x08:

            if (led_state == pattern_4){

                counter_pattern_4 = 255;

            }

            led_state = pattern_4;

            break;

        case 0x09:

            if (led_state == pattern_5){

                step_pattern_5 = 0;

            }

            led_state = pattern_5;

            break;

        case 0x10:

            if (led_state == pattern_6)
            {

                step_pattern_6 = 0;

            }

            led_state = pattern_6;

            break;

        case 0x11:

            if (led_state == pattern_7)
            {

                step_pattern_7 = 0;

            }

            led_state = pattern_7;

            break;
      
    }

}
