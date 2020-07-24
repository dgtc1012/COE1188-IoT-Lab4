/*
 * I2C.h
 *
 *  Created on: Mar 2, 2018
 *      Author: Dgtc1
 */

#ifndef I2C_H_
#define I2C_H_
#include "msp432p401r.h"

#define SLAVE_ADDR 0b1001000

void I2C_Init();
uint16_t I2C_TX_Byte(uint8_t address, uint8_t data);
uint16_t I2C_RX_2_Bytes(uint8_t slave_address);


#endif /* I2C_H_ */
