#include "I2C.h"
#include "ClockSystem.h"
#include <stdio.h>

void SysTick_Init(){
	SYSTICK_STCSR = 0; // 1) disable SysTick during setup
	SYSTICK_STRVR = 0x00ffffff; // 2) delay by correct number of clock cycles
	SYSTICK_STCVR = 0; // 3) any write to current clears it
	SYSTICK_STCSR = 0x00000005; // 4) enable SysTick with core clock
}

void Initialize_LEDs(){
	//Port 2 has the red and green LEDs for the walk light
	P2SEL0 |= 0x00;
	P2SEL1 |= 0x00;
	P2REN |= 0x01;
	P2DIR |= 0x01;

	P2OUT &= ~(BIT0);
}

void Initialize_Button(){
	P1SEL0 &= ~BIT1;
	P1SEL1 &= ~BIT1;
	P1REN |= BIT1;
	P1DIR &= ~BIT1;
}

//converts binary value to degrees celcius
double Binary_to_Temp(uint16_t binary){
	double result;
	int temp;
	//if > 0 then number is negative
	if(binary & 0x0800){
		result = (((binary & 0x0FFF) ^ 0xFFFF) + 1) * 0.0625 * (-1);
	}
	//else number is positive
	else{
		temp = (binary & 0x0FFF);
		result = ((double)temp) * 0.0625;
	}
	return result;
}

void main(){
	Clock_Init(DCO12MHz);
	SysTick_Init();
	Initialize_LEDs();
	Initialize_Button();
	I2C_Init();

	uint16_t sensor_feedback;
	uint8_t slave_address = 0b10010000;
	uint8_t pointer_reg = 0x00;
	double temp;

	//TODO: send byte to temp sensor specifying the read register
	uint16_t error = I2C_TX_Byte(slave_address, pointer_reg);
	while(error != 0)
	{
		I2C_Init();
		error = I2C_TX_Byte(slave_address, pointer_reg);
	}

	while(1)
	{
		if(!(P1IN & BIT1)){
			uint16_t error = I2C_TX_Byte(slave_address, pointer_reg);
			while(error != 0)
			{
				I2C_Init();
				error = I2C_TX_Byte(slave_address, pointer_reg);
			}

			sensor_feedback = I2C_RX_2_Bytes(slave_address);
			temp = Binary_to_Temp(sensor_feedback);
			printf("Temperature: %f\n", temp);
			if(temp > 25){
				P2OUT |= BIT0;
			}
			else{
				P2OUT &= ~BIT0;
			}
		}
	}
}
