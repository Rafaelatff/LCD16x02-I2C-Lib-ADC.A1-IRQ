/*
 * I2C.h
 *
 *  Created on: 10 Dec 2021
 *      Author: rafaela
 */

#ifndef I2C_H_
#define I2C_H_

void start_I2C_B1(void);
void start_LCD_I2C(void);
void send_LCD_nibble_I2C(char data);
void send_LCD_byte_I2C(char RS, char data);
void send_LCD_text_I2C(char text[], char line, char pos);

#endif /* I2C_H_ */
