#include "nRF24L01.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


//*********************************************

/**************************************************
* Function: nRF24RW();
*
* Description:
* Writes one unsigned char to nRF24L01, and return the unsigned char read
* from nRF24L01 during write, according to SPI protocol
**************************************************/
unsigned char nRF24L01::RW(unsigned char Byte)
{
	unsigned char i;
	for (i = 0; i<8; i++)                      // output 8-bit
	{
		if (Byte & 0x80)
		{
			digitalWrite(this->MOSI, 1);
		}
		else
		{
			digitalWrite(this->MOSI, 0);
		}
		digitalWrite(this->SCK, 1);
		Byte <<= 1;                         // shift next bit into MSB..
		if (digitalRead(this->MISO) == 1)
		{
			Byte |= 1;       	                // capture current this->MISO bit
		}
		digitalWrite(this->SCK, 0);
	}
	return(Byte);           	        // return read unsigned char
}
/**************************************************/

/**************************************************
* Function: nRF24RW_Reg();
*
* Description:
* Writes value 'value' to register 'reg'
/**************************************************/
unsigned char nRF24L01::RW_Reg(unsigned char reg, unsigned char value)
{
	unsigned char status;

	digitalWrite(this->CSN, 0);                   // this->CSN low, init SPI transaction
	status = RW(reg);                   // select register
	RW(value);                          // ..and write value to it..
	digitalWrite(this->CSN, 1);                   // this->CSN high again

	return(status);                   // return nRF24L01 status unsigned char
}
/**************************************************/

/**************************************************
* Function: nRF24Read();
*
* Description:
* Read one unsigned char from nRF24L01 register, 'reg'
/**************************************************/
unsigned char nRF24L01::Read(unsigned char reg)
{
	unsigned char reg_val;

	digitalWrite(this->CSN, 0);           // this->CSN low, initialize SPI communication...
	RW(reg);                   // Select register to read from..
	reg_val = RW(0);           // ..then read register value
	digitalWrite(this->CSN, 1);          // this->CSN high, terminate SPI communication

	return(reg_val);               // return register value
}
/**************************************************/

/**************************************************
* Function: nRF24Read_Buf();
*
* Description:
* Reads 'unsigned chars' #of unsigned chars from register 'reg'
* Typically used to read RX payload, Rx/Tx address
/**************************************************/
unsigned char nRF24L01::Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
	unsigned char status, i;

	digitalWrite(this->CSN, 0);                  // Set this->CSN low, init SPI tranaction
	status = RW(reg);       	    // Select register to write to and read status unsigned char

	for (i = 0; i<bytes; i++)
	{
		pBuf[i] = RW(0);    // Perform nRF24RW to read unsigned char from nRF24L01
	}

	digitalWrite(this->CSN, 1);                   // Set this->CSN high again

	return(status);                  // return nRF24L01 status unsigned char
}
/**************************************************/

/**************************************************
* Function: nRF24Write_Buf();
*
* Description:
* Writes contents of buffer '*pBuf' to nRF24L01
* Typically used to write TX payload, Rx/Tx address
/**************************************************/
unsigned char nRF24L01::Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
	unsigned char status, i;

	digitalWrite(this->CSN, 0);                  // Set this->CSN low, init SPI tranaction
	status = RW(reg);             // Select register to write to and read status unsigned char
	for (i = 0; i<bytes; i++)             // then write all unsigned char in buffer(*pBuf)
	{
		RW(*pBuf++);
	}
	digitalWrite(this->CSN, 1);                   // Set this->CSN high again
	return(status);                  // return nRF24L01 status unsigned char
}
/**************************************************/