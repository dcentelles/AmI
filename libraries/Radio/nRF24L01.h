#ifndef NRF24L01_h
#define NRF24L01_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "API.H"

//---------------------------------------------
#define TX_ADR_WIDTH    5   
// 5 unsigned chars TX(RX) address width
#define TX_PLOAD_WIDTH  32  
// 20 unsigned chars TX payload
//---------------------------------------------

class nRF24L01
{
public:
	nRF24L01() :
		CE		(8),
		CSN		(9),
		SCK		(10),
		MOSI	(11),
		MISO	(12),
		IRQ		(13) {};

	
	void InitSPI(
		int ce = 8,
		int csn = 9,
		int sck = 10,
		int mosi = 11,
		int miso = 12,
		int irq = 13)
	{
		this->CE = ce;
		this->SCK = sck;
		this->CSN = csn;
		this->MOSI = mosi;
		this->MISO = miso;
		this->IRQ = irq;
		
		pinMode(this->CE, OUTPUT);
		pinMode(this->SCK, OUTPUT);
		pinMode(this->CSN, OUTPUT);
		pinMode(this->MOSI, OUTPUT);
		pinMode(this->MISO, INPUT);
		pinMode(this->IRQ, INPUT);
	}
	int CE, SCK, CSN, MOSI, MISO, IRQ;
	unsigned char RW(unsigned char Byte);
	unsigned char RW_Reg(unsigned char reg, unsigned char value);
	unsigned char Read(unsigned char reg);
	unsigned char Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes);
	unsigned char Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes);
};




#endif