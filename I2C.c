#include "I2C.h"

void I2C_Init(){

	UCB0CTLW0 |= UCSWRST; 	//turn on reset

	//set p6 ports 4 and 5 to be SDA and SCL respectively
	P1SEL0 |= (BIT6|BIT7);
	P1SEL1 |= ~(BIT6|BIT7);


	//set the USCI control register 0
	UCB0CTLW0 |= UCMST; 	//set to master
	UCB0CTLW0 |= UCMODE_3;	//I2C mode
	UCB0CTLW0 |= UCSYNC;	//synchronous
	UCB0CTLW0 |= UCSSEL_3;	//clock source is SMCLK = 12 MHz
	UCB0CTLW0 &= ~UCA10;	//own address is 7-bit address
	UCB0CTLW0 &= ~UCSLA10;	//slave address is 7-bit address
	UCB0CTLW0 &= ~UCMM;		//single master environment

	//set control register 1
	UCB0CTLW1 |= UCCLTO_3;
	UCB0CTLW1 |= UCASTP_2;
	UCB0CTLW1 |= UCGLIT_0;

	//set byte count register to 1 byte for auto stop condition
	UCB0TBCNT = 1;

	//set bit rate control info
	UCB0BRW = 240; //12MHz clock / 240 = 50 kbps or 0.05 MHz

	//clear reset bit
	UCB0CTLW0 &= ~UCSWRST; //disable reset

	//disable interrupts
	UCB0IE = 0x00;

}

//transmit 1 byte
uint16_t I2C_TX_Byte(uint8_t address, uint8_t data){

	//while bus is busy, wait (determined by status word register)
	while((UCB0STATW & UCBBUSY))
	{

	}

	//set interface into reset mode for config
	UCB0CTLW0 |= UCSWRST; 	//turn on reset

	//set to generate stop conditions after 2 bytes
	UCB0TBCNT = 1;

	//turn off reset mode
	UCB0CTLW0 &= ~UCSWRST; //disable reset

	//set slave address
	UCB0I2CSA = address;

	//set to transmit mode, transmit start condition
	UCB0CTLW0 |= UCTR; //set as transmitter
	UCB0CTLW0 |= UCTXSTT; //generate start condition
	UCB0CTLW0 &= ~UCTXSTP; //dont generate stop condition

	//UCTXSTT is cleared once start and address are sent, busy wait till this happens
	while(UCB0CTLW0 & UCTXSTT)
	{

	}

	//send out data byte (set transmit data buffer reg)
	UCB0TXBUF = data;

	//busy wait while buffer has content (interrupt flag register TXIFGO
	while(!(UCB0IFG & UCTXIFG0))
	{
		//if no acknowledgement, no slave device (check from sending slave addr)
		//UCNACKIFG flag in IFG register
		if(UCB0IFG & UCNACKIFG){
			I2C_Init();
			return UCB0IFG;
		}
	}

	UCB0IFG &= ~UCRXIFG0;

	//while bus is busy, wait (determined by status word register)
	while((UCB0STATW & UCBBUSY))
	{
		//if no acknowledgement, no slave device (check from sending data)
		//UCNACKIFG flag in IFG register
		if(UCB0IFG & UCNACKIFG){
			I2C_Init();
			return UCB0IFG;
		}
	}

	return 0;
}


//receive two bytes
uint16_t I2C_RX_2_Bytes(uint8_t slave_address){

	uint16_t result;

	//set interface into reset mode for config
	UCB0CTLW0 |= UCSWRST; 	//turn on reset

	//set to generate stop conditions after 2 bytes
	UCB0TBCNT = 2;

	//turn off reset mode
	UCB0CTLW0 &= ~UCSWRST; //disable reset

	//set slave address, set last bit to 1 to read
	UCB0I2CSA = slave_address >> 1;

	//while bus is busy, wait (determined by status word register)
	while((UCB0STATW & UCBBUSY))
	{

	}

	//set to transmit mode, transmit start condition
	UCB0CTLW0 &= ~UCTR; //set as receiver
	UCB0CTLW0 |= UCTXSTT; //generate start condition
	UCB0CTLW0 &= ~UCTXSTP; //dont generate stop condition

	//UCTXSTT is cleared once start and address are sent, busy wait till this happens
	while(UCB0CTLW0 & UCTXSTT)
	{

	}

	//wait for first byte
	while(!(UCB0IFG & UCRXIFG0)){
		//if no acknowledgement, no slave device (check from sending slave addr)
		//UCNACKIFG flag in IFG register
		if(UCB0IFG & UCNACKIFG){
			I2C_Init();
			return 0xFFFF;
		}
	}

	result = UCB0RXBUF << 4;

	//clear RX buffer to clear receive interrupt flag?
	UCB0RXBUF = 0x0000;
	UCB0IFG &= ~UCRXIFG0;

	//wait for second byte
	while(!(UCB0IFG & UCRXIFG0)){
		//if no acknowledgement, no slave device (check from sending slave addr)
		//UCNACKIFG flag in IFG register
		if(UCB0IFG & UCNACKIFG){
			I2C_Init();
			return 0xFFFF;
		}
	}

	result = result | ((UCB0RXBUF >> 4) & 0x0F);
	return result;
}
