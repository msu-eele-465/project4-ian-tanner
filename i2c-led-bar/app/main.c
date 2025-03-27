int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    //---------------- Configure UCB0 I2C ----------------

    // Configure P1.2 (SDA) and P1.3 (SCL) for I2C on MSP430FR2355
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);

    // Enable pull-up resistors on SDA (P1.2) and SCL (P1.3)
    P1REN |= BIT2 | BIT3;
    P1OUT |= BIT2 | BIT3;

    // LED box
    P3DIR |= LED1 | LED2 | LED3 | LED4 | LED5 | LED6 | LED7 | LED8;
    P3OUT &= ~(LED1 | LED2 | LED3 | LED4 | LED5 | LED6 | LED7 | LED8);

    // Data LED
    P4DIR |= LED1;
    P4OUT &= ~LED1;

    // Configure Timers
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;

    TB0CCR0 = transition;
    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;

    // I2C Configuration
    UCB0CTLW0 = UCSWRST;
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;
    UCB0I2COA0 = 0x01 | UCOAEN;
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCRXIE0;

    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    // Main loop does nothing, just handles interrupts
    while(1){

        delay_loop();

    }

    return 0;
}

// Timer B0 interrupt
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void)
{
    uint8_t pattern = 0;

    switch (led_state)
    {
        // All LEDs off
        case pattern_off:
            pattern = 0x00;
            break;

        // 10101010
        case pattern_0:
            pattern = 0x55;
            step_pattern_0++;
            if (step_pattern_0 == 8) step_pattern_0 = 0;  // Reset after 8 steps
            break;

        case pattern_1:
            // Alternates between 10101010 and 01010101
            pattern = (step_pattern_1++ % 2 == 0) ? 0x55 : 0xAA;
            if (step_pattern_1 == 8) step_pattern_1 = 0;
            break;

        case pattern_2:
            // Up counter for pattern_2
            pattern = counter_pattern_2_0++;
            if (counter_pattern_2_0 > 255) counter_pattern_2_0 = 0;
            break;

        case pattern_3:
            // Moves 2 LEDs out and in from 00011000 to 00100100 to 01000010 to 10000001 before reversing
            switch (step_pattern_3) {
                case 0: pattern = 0x18; break;
                case 1: pattern = 0x24; break;
                case 2: pattern = 0x42; break;
                case 3: pattern = 0x81; break;
                case 4: pattern = 0x42; break;
                case 5: pattern = 0x24; break;
                case 6: pattern = 0x18; break;
            }
            step_pattern_3 = (step_pattern_3 + 1) % 6;
            break;

        case pattern_4:
            // Down counter for pattern_4
            pattern = counter_pattern_4_0--;
            if (counter_pattern_4_0 == 0) counter_pattern_4_0 = 255;
            break;

        case pattern_5:
            // Rotate left one LED
            pattern = (1 << step_pattern_5);
            step_pattern_5 = (step_pattern_5 + 1) % 8;
            break;

        case pattern_6:
            // Rotate right one off LED leaving others on
            pattern = (0xFF & ~(1 << step_pattern_6));
            step_pattern_6 = (step_pattern_6 + 1) % 8;  // After 7 steps, reset to 0
            break;

        case pattern_7:
            // 00000001, 00000011, 00000111, etc.
            pattern = (1 << (step_pattern_7 + 1)) - 1;
            step_pattern_7 = (step_pattern_7 + 1) % 8;
            break;
    }

    update_leds(pattern);
    TB0CCTL0 &= ~CCIFG;
}

// I2C RX interrupt
#pragma vector = EUSCI_B0_VECTOR
__interrupt void ISR_I2C_B0(void) {
    read_in = UCB0RXBUF;

    // If a pattern is reselected while it is already active, reset it
    if (read_in == led_state)
    {

        // Reset the pattern
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
                step_pattern_0 = 0;
                break;

            case 0x05:
                led_state = pattern_1;
                step_pattern_1 = 0;
                break;

            case 0x06:
                led_state = pattern_2;
                counter_pattern_2_0 = 0;
                break;
            case 0x07:
                led_state = pattern_3;
                step_pattern_3 = 0;
                break;

            case 0x08:
                led_state = pattern_4;
                counter_pattern_4_0 = 255;
                break;

            case 0x09:
                led_state = pattern_5;
                step_pattern_5 = 0;
                break;

            case 0x10:
                led_state = pattern_6;
                step_pattern_6 = 0;
                break;

            case 0x11:
                led_state = pattern_7;
                step_pattern_7 = 0;
                break;

        }
    }

    else
    {

        // Save the current state of the active pattern before switching
        switch (led_state) {
            case pattern_0:
                step_pattern_0 = step_pattern_0;
                break;

            case pattern_1:
                step_pattern_1 = step_pattern_1;
                break;

            case pattern_2:
                counter_pattern_2_0 = counter_pattern_2_0;
                break;

            case pattern_3:
                step_pattern_3 = step_pattern_3;
                break;

            case pattern_4:
                counter_pattern_4_0 = counter_pattern_4_0;
                break;

            case pattern_5:
                step_pattern_5 = step_pattern_5;
                break;

            case pattern_6:
                step_pattern_6 = step_pattern_6;
                break;

            case pattern_7:
                step_pattern_7 = step_pattern_7;
                break;

            default:
                break;

        }

        // Now select the new pattern
        switch (read_in)
        {

            case 0x01:
                led_state = pattern_off;
                break;

            case 0x02:
                transition -= 8192;
                if (transition < 8192)
                {
                    transition = 8192;
                }
                break;

            case 0x03:
                transition += 8192;
                if (transition > 57344)
                {
                    transition = 57344;
                }
                break;

            case 0x04:
                led_state = pattern_0;
                break;

            case 0x05:
                led_state = pattern_1;
                break;

            case 0x06:
                led_state = pattern_2;
                break;

            case 0x07:
                led_state = pattern_3;
                break;

            case 0x08:
                led_state = pattern_4;
                break;

            case 0x09:
                led_state = pattern_5;
                break;

            case 0x10:
                led_state = pattern_6;
                break;

            case 0x11:
                led_state = pattern_7;
                break;

        }

    }

    P4OUT |= LED1;


}

