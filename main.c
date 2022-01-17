
//CCS version 10.2.0.00009
#include <msp430.h>
#include <I2C.h>
#include <ADC.h>
#include <stdio.h>

int ADC_value;

char text1[17]="Oi pai!";
char text2[17]="A1 = ";
char text3[10];

//Lets program the ADC (A1) -> pin P1.1
//ADC Single-Channel Single-Conversion Mode
//Default CPU clock is MCLK = SMCLK = default DCODIV ~1MHz. Low frequency peripheral clock ACLK = default REFO ~32768Hz. ADC dedicated clock MODCLK = ~5MHz.
//Reading Voltage w/ Conversion-Complete Polling
//Polling would mean you spinning in a loop wait for the conversion to complete, performing no other useful work.
//With multiple channels, and interleaving, the model is to use DMA to capture the data into an array, ideally one containing 100's of samples, and calling an interrupt to process once complete.


int main(void)
{
     WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //Set LED1 for test routines
    // Set P1.0 as output direction [J7 needs to be connected]
    // LED White: Port P1 / Pin 0
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1DIR |= BIT0; // Set LED1 as Output
    P1OUT &= ~BIT0; // turn off LED1

    PM5CTL0 &= ~LOCKLPM5;       //to apply the GPIO configurations disable the LOCKLPM5 bit in PM5CTL0
    //unlock the GPIO high-impedance mode by resetting the LOCKLPM5 bit

    start_I2C_B1();
    start_LCD_I2C();

    start_ADC();

    send_LCD_text_I2C(text1,1,0);
    send_LCD_text_I2C(text2,2,0);


    ADCCTL0 |= ADCENC | ADCSC; // Enable and start convertion

    while (1){
        //while ((ADCIFG & ADCIFG0)==0); // sits and waits for convertion to be finished

        //test routine
        if (ADC_value > 100){
            P1OUT |= BIT0;
        } else {
            P1OUT &= ~BIT0;
        }

        sprintf(text3,"%d",ADC_value);
        send_LCD_text_I2C(text3,2,5);
        __delay_cycles(100000);
    }
    return 0;
}





