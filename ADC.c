/*
 * ADC.c
 *
 *  Created on: 26 Dec 2021
 *      Author: rafaela
 */

#include <msp430.h>

int ADC_value;

//// ADC interrupt service routine
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void){

    //Gets the result of the ADC
    ADC_value = ADCMEM0;

    ADCCTL0 |= ADCENC | ADCSC; // start next conversion;
}




void start_ADC(void){
    // Configure ADC A1 pin, shared with P1.1 -> P1SELx = 11
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;
    //SYSCFG2 |= ADCPCTL1;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure ADC10
    ADCCTL0 &= ~ADCSHT; // this and the line bellows set conv clock cycles to 16 (default is 8) = 10
    ADCCTL0 |= ADCSHT_2 | ADCON; // ADCON, S&H=16 ADC clks

    ADCCTL1 |= ADCSSEL_2; // chooses SMCLK (1MHz)
    ADCCTL1 |= ADCSHP; // sample signal source = sampling timer

    ADCCTL2 &= ~ADCRES; // to select 12-bit resolution, LSB must be set to 0
    ADCCTL2 |= ADCRES_2; // and the second one to 1 (10) - 12-bit conversion results

    //ADCIE |= ADCIE0; // Enable ADC conv complete interrupt, commented, since I am using pooling
    ADCMCTL0 |= ADCINCH_1; // A1 ADC input select;

    // Configure voltage reference for ADC conversion
    // not using right now
    //ADCMCTL0 |= ADCSREF_1; // To use  Vref is internal 1.5V
    //PMMCTL0_H = PMMPW_H;   // Unlock the PMM registers
    //PMMCTL2 |= INTREFEN;   // Enable internal reference 1.5V
    //__delay_cycles(400);   // Delay for reference settling

    //--Setup IRQ
    ADCIE |= ADCIE0; // Conversion complete IRQ, local enable
    __enable_interrupt(); // enable maskables


    //move this part to main after it starts to work
    /*
    send_LCD_text_I2C(text1,1,0);
    send_LCD_text_I2C(text2,2,0);

    while (1){
        ADCCTL0 |= ADCENC | ADCSC; // Enable and start convertion
        while ((ADCIFG & ADCIFG0)==0); // sits and waits for convertion to be finished

        ADC_value = ADCMEM0;
        //test routine
        if (ADC_value > 100){
            P1OUT |= BIT0;
        } else {
            P1OUT &= ~BIT0;
        }

        sprintf(text3,"%d",ADC_value);
        send_LCD_text_I2C(text3,2,5);
        __delay_cycles(100000);
        */
    }


