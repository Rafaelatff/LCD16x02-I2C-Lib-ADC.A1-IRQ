/*
 * I2C.c
 *
 *  Created on: 10 Dec 2021
 *      Author: rafaela
 */
#include <msp430.h>
#include <string.h> //to use strlen() function


#define SLAVE_ADDR  0x27 //0010 0111 - GPIO expanser address
int i;
char LCDmessage = 0x08;


//-------------ISR-----------------
#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void){
    UCB1TXBUF = LCDmessage;
}


//*         About I2C            *//
//SDA pin 14 (P1.2) - UCB0SDA
//SCL pin 15 (P1.3) - UCB0SCL

//SDA pin 38 (P3.2) - UCB1SDA
//SCL pin 39 (P3.6) - UCB1SCL

// Slave name: PCF8574
// Slave addres: 0 1 0 0 A2 A1 A0 R/~W= 0x27 (7 bits | without resistor Ax = 1)
// A2 A1 A0 are 1
// If the R/W bit is high, the data from this device are the values
// read from the P port. If the R/W bit is low, the data are from the master, to be output to the P port.



//*         About LCD            *//
/*---- Connections ----*/
//  P0 = RS (LSB) = xxxx xxxP0 = 0x01
//  P1 = RW = xxxx xxP1x = 0x02
//  P2 = E = xxxx xP2xx = 0x04
//  P3 = Display light (1 = on, 0 = off) = xxxx P3xxx = 0x08
//  P4 = D4 = xxxP4 xxxx = 0x10
//  P5 = D5 = xxP5x xxxx = 0x20
//  P6 = D6 = xP6xx xxxx = 0x40
//  P7 = D7 (MSB) = P7xxx xxxx = 0x80
// Send an enable signal - Falling edge  -> 0010xxxx
//  RW (LCD) = GND (MSP) // Read = 1 and Write = 0
// RS (LCD) = P4.1 (MSP) // Text = 1 and Command = 0

void start_I2C_B1(void);
void start_LCD_I2C(void);
void send_LCD_nibble_I2C(char data);
void send_LCD_byte_I2C(char RS, char data);
void send_LCD_text_I2C(char text[], char line, char pos);

void start_I2C_B1(void){

    // always put the eUSCI module into its reset state
    UCB1CTLW0 = UCSWRST;                      //Put B0 in SW reset

    //configure the eUSCI module as a synchronous I2C peripheral, master mode, using the SMCLK source
    UCB1CTLW0 |= UCSSEL__SMCLK; // UCSSEL_3 = SMCLK,
    //UCB0CTLW0 |= UCMODE_3;
    // The SMCLK is configured for 10MHz, so we must divide it down to meet the 100kHz I2C clock requirement
    UCB1BRW = 10;                            // fSCL = SMCLK/10 = ~100kHz // set prescalar to 10

    UCB1CTLW0 |= UCMODE_3; //put into I2C mode
    UCB1CTLW0 |= UCMST; //set as master
    UCB1CTLW0 |= UCTR; //put into Tx mode (to write)
    UCB1I2CSA = SLAVE_ADDR; //set slave address

    UCB1CTLW1 |= UCASTP_2; //auto STOP mode
    //UCB0TBCNT = 1; // transfer buffer count = 1 (1 byte)
    UCB1TBCNT = 1; //not it has the number of bytes I allocate in this variable

    //Config PIN SEL  - for UCB0SDA e UCB0SCL PxSELx = 01
    //SDA pin 14 (P1.2) - UCB0SDA
    //SCL pin 15 (P1.3) - UCB0SCL
    //P1SEL0 |= BIT2 | BIT3;
    //P1SEL1 &= ~(BIT2 | BIT3);

    //To use UCB1SDA e UCB1SCL
    //SDA pin 38 (P3.2) - UCB1SDA
    //SCL pin 39 (P3.6) - UCB1SCL
    P3SEL0 |= BIT2 | BIT6;
    P3SEL1 &= ~(BIT2 | BIT6);

    //Obs.: To set an interrupt for I2C (in case MSP is slave), set an ordinary GPIO;

    PM5CTL0 &= ~LOCKLPM5;  //to apply the GPIO configurations disable the LOCKLPM5 bit in PM5CTL0

    UCB1CTLW0 &= ~UCSWRST;  //Take B0 out of SW RST

    //Enable B0 TX0 IRQ
    UCB1IE |= UCTXIE0;  //local enable for TX0
    __enable_interrupt(); //enable maskables
}

void start_LCD_I2C(void){

    LCDmessage = 0x08;
    UCB1CTLW0 |= UCTXSTT;

    send_LCD_nibble_I2C(0x02); //Tell the LCD to work with 4 bits
    for (i=0;i<500;i++); //delay 2 ms

    send_LCD_byte_I2C(0,0x28); // 0x0011.1000 FUNCTION SET - 4 bits, 2 lines, 5x7 dots
    send_LCD_byte_I2C(0,0x0F); // 0x0000.1111 DISPLAY CONTROL - Display on, no cursor
    send_LCD_byte_I2C(0,0x01); // CLEAN LCD
    send_LCD_byte_I2C(0,0x06); // ENTRY MODE SET - Shift cursor to the right each written character
    for (i=0;i<500;i++); //delay 2 ms

}

void send_LCD_text_I2C(char text[], char line, char pos){
    char a;
    char text_size = strlen(text); //read string size

    if(line==1)line=0x80;   //Set line 1 of LCD = 0x8*
        else if (line==2)line=0xC0;  //Set line 2 of LCD = 0xC*

    send_LCD_byte_I2C(0,line+pos); //Send line and position command

    for(a=0;a<text_size;a++){    //Stay in loop while send all the texts
        send_LCD_byte_I2C(1,text[a]);     // Send each character one by one to LCD
        }
}

void send_LCD_nibble_I2C(char data){
    if(data & 0x01) LCDmessage |= 0x10;
        else LCDmessage &= ~0x10;
    if(data & 0x02) LCDmessage |= 0x20;
        else LCDmessage &= ~0x20;
    if(data & 0x04) LCDmessage |= 0x40;
        else LCDmessage &= ~0x40;
    if(data & 0x08) LCDmessage |= 0x80;
        else LCDmessage &= ~0x80;

    //Send an enable signal - Falling edge
    LCDmessage |= 0x04;
    UCB1CTLW0 |= UCTXSTT;
    for (i=0;i<500;i++); //delay 2 ms
    LCDmessage &= ~0x04;
    UCB1CTLW0 |= UCTXSTT;
    for (i=0;i<500;i++); //delay 2 ms
}

void send_LCD_byte_I2C(char RS, char data){
    if (RS==0) LCDmessage &= ~0x01;
        else LCDmessage |= 0x01;

    //set EN low
    LCDmessage &= ~0x04;
    UCB1CTLW0 |= UCTXSTT;

    send_LCD_nibble_I2C(data>>4); // Send high part of data
    send_LCD_nibble_I2C(data & 0x0f);// Clean high part of data and send low part
    for (i=0;i<500;i++); //delay 2 ms
}

