#include "Radio.h"
#include <Arduino.h>
#include "nRF24L01.h"

namespace ami
{

#define BRD_PIPE_ADREG RX_ADDR_P0
#define OWN_PIPE_ADREG RX_ADDR_P1
#define ACK_PIPE_ADREG RX_ADDR_P2

#define BRD_PIPE 0x1
#define OWN_PIPE 0x2
#define ACK_PIPE 0x4

#define STATE_DELAY 50

	Radio::Radio()
	{
	}


	Radio::~Radio()
	{
	}

	void Radio::Init(bool autoAck,
		int ce,
		int csn,
		int sck,
		int mosi,
		int miso,
		int irq)
	{
		_available = 0;
		_autoAck = autoAck;

		_device.InitSPI(ce, csn, sck,mosi,miso,irq);

		_init_io();

		digitalWrite(_device.CE, 0); //make sure device is disabled

		SetRxAddr(ADR); //Establecer la address del dispositivo

		_device.Write_Buf(WRITE_REG + BRD_PIPE_ADREG, (unsigned char*)BRD_ADDR, TX_ADR_WIDTH); //Set broadcast addres

		_device.RW_Reg(WRITE_REG + EN_RXADDR, 0); //Desactivar todas las pipes
		_device.RW_Reg(WRITE_REG + DYNPD, ACK_PIPE | OWN_PIPE | BRD_PIPE);     //Activar DPL en las pipes

		if(_autoAck)
			_device.RW_Reg(WRITE_REG + EN_AA, OWN_PIPE ); // Enable Auto.Ack solo en la pipe propia
		else
			_device.RW_Reg(WRITE_REG + EN_AA, 0); // Disable Auto.Ack en todas las pipes


		_device.RW_Reg(WRITE_REG + SETUP_RETR, 0x1a); // 500us + 86us, 10 retrans...
		SetChannel(40);
		_device.RW_Reg(WRITE_REG + RF_SETUP, 0x27);   // 2 Mbps, 2 MHz bandwidth

		_device.RW_Reg(WRITE_REG + FEATURE, 0x04);  //Activar DPL (EN_DPL bit) DPL = Dynamic Payload Length (en registro FEATURE)
		
		digitalWrite(_device.CE, 1); //enable device
	}

	void Radio::SetChannel(int channel)
	{
		_device.RW_Reg(WRITE_REG + RF_CH, channel);  // Select RF channel
	}

	int Radio::Available()
	{
		if (_available)
			return _available;

		if (_readyForRead())
		{
			return _getRxPldWidth();
		}
		return 0;
	}

	int Radio::_getRxPldWidth()
	{
		return _device.Read(0x60);
	}

	void Radio::TxMode()
	{
		digitalWrite(_device.CE, 0); //disable radio

		if(_autoAck)
			_device.RW_Reg(WRITE_REG + EN_RXADDR, ACK_PIPE); //Activar pipe de ack
		else
			_device.RW_Reg(WRITE_REG + EN_RXADDR, 0); //Desactivar pipe de ack

		_device.RW_Reg(WRITE_REG + CONFIG, 0x0e); // Set PWR_UP bit, enable CRC(2 unsigned chars) & Prim:TX. MAX_RT & TX_DS enabled..

		digitalWrite(_device.CE, 1); //enable radio
		delay(STATE_DELAY); //Esperamos un tiempo para asegurarnos de que el dispositivo este en modo TX
	}
	
	void Radio::RxMode(void)
	{
		delay(STATE_DELAY); //Esperamos un tiempo prudencial por si hay datos transmitiendose
		digitalWrite(_device.CE, 0); //disable radio
		
		_device.RW_Reg(WRITE_REG + EN_RXADDR, OWN_PIPE | BRD_PIPE); //Activar pipe propia y de broadcast
		_device.RW_Reg(WRITE_REG + CONFIG, 0x0f); // Set PWR_UP bit, enable CRC(2 unsigned chars) & Prim:RX. RX_DR enabled..

		digitalWrite(_device.CE, 1); //enable radio
		delay(STATE_DELAY); //Esperamos un tiempo para asegurarnos de que el dispositivo este en modo RX
	}

	void Radio::SetRxAddr(const uint8_t *addr)
	{
		memcpy(ADR, addr, ADR_WIDTH);
		_setRxAddr();
	}

	void Radio::_setRxAddr()
	{
		_device.Write_Buf(WRITE_REG + OWN_PIPE_ADREG, (unsigned char*)ADR, ADR_WIDTH);
	}

	void Radio::SetRxAddr(uint8_t ad0, uint8_t ad1, uint8_t ad2, uint8_t ad3, uint8_t ad4)
	{
		ADR[0] = ad0;
		ADR[1] = ad1;
		ADR[2] = ad2;
		ADR[3] = ad3;
		ADR[4] = ad4;
		_setRxAddr();
	}

	void Radio::BroadcastMode(bool broadcast)
	{
		if (broadcast)
		{
			_setTxAddr(BRD_ADDR);
		}
		else
		{
			_setTxAddr(TX_ADR);
		}
	}

	void Radio::SetTxAddr(const uint8_t *addr)
	{
		memcpy(TX_ADR, addr, ADR_WIDTH);
		_setTxAddr(TX_ADR);
	}

	void Radio::_setTxAddr(const uint8_t * addr)
	{
		_device.Write_Buf(WRITE_REG + TX_ADDR, (unsigned char*)addr, ADR_WIDTH); // Writes TX_Address to _device.L01
		_device.Write_Buf(WRITE_REG + ACK_PIPE_ADREG, (unsigned char*)addr, ADR_WIDTH); // RX_Addr0 same as TX_Adr for Auto.Ack
	}

	void Radio::SetTxAddr(uint8_t ad0, uint8_t ad1, uint8_t ad2, uint8_t ad3, uint8_t ad4)
	{
		TX_ADR[0] = ad0;
		TX_ADR[1] = ad1;
		TX_ADR[2] = ad2;
		TX_ADR[3] = ad3;
		TX_ADR[4] = ad4;
		_setTxAddr(TX_ADR);
	}

	void Radio::Read(void * dst, unsigned int req) //Blocking call
	{
		if (req <= _available)
		{
			memcpy(dst, _buff, req);
			_available = _available - req;
			memcpy(_buff, _buff + req, _available);
			Serial.print("Se han pedido: "); Serial.print(req); Serial.print(" ; ");
			Serial.print("Sobran en el buffer: "); Serial.println(_available);
			return;
		}
		else
		{
			uint8_t * ptr = (uint8_t *)dst;
			memcpy(ptr, _buff, _available);
			ptr += _available;
			unsigned int left = req - _available;
			//Serial.print("Se han leido del buffer: "); Serial.println(_available);
			_available = 0;
			//Serial.print("Se han pedido: "); Serial.print(req); Serial.print(" ; ");
			//Serial.print("Faltan: "); Serial.println(left);
			while (left > 0)
			{
				while (!_readyForRead());
				uint8_t dataLength;
				dataLength = _device.Read(0x60);
				_device.Read_Buf(RD_RX_PLOAD, _buff, dataLength);
				if (dataLength > TX_PLOAD_WIDTH) continue; //Esto es por si acaso...
				if (dataLength <= left)
				{
					memcpy(ptr, _buff, dataLength);
					ptr += dataLength;
					left -= dataLength;
				}
				else
				{
					memcpy(ptr, _buff, left);
					_available = dataLength - left;
					memcpy(_buff, _buff + left, _available);
					left = 0;
				}
				/*
				Despues de leer, siempre resetear STATUS register
				Si no se hace, no funciona correctamente... Consultar
				Datasheet
				*/
				_status = _device.Read(STATUS);
				_device.RW_Reg(WRITE_REG + STATUS, _status);
			}
		}

	}

	int Radio::_readyForRead()
	{
		_status = _device.Read(STATUS);
		_fifostatus = _device.Read(FIFO_STATUS);
		//En teoria, con comprobar RX_EMPTY es suficiente...
		boolean result = ((_status&RX_DR) || !(_fifostatus & RX_EMPTY)) && (_fifostatus & TX_EMPTY);

		_device.RW_Reg(WRITE_REG + STATUS, _status); //RESETEAMOS LOS BITS RX_DR, (y TX_DR y MAX_RT, aunque no los tenemos en cuenta)
		return result;
	}

	//**************************************************
	// Function: init_io();
	// Description:
	// flash led one time,chip enable(ready to TX or RX Mode),
	// Spi disable,Spi clock line init high
	//**************************************************
	void Radio::_init_io(void)
	{
		digitalWrite(_device.IRQ, 0);
		digitalWrite(_device.CE, 0);			// chip enable
		digitalWrite(_device.CSN, 1);                 // Spi disable	
	}

	void Radio::_clearStatus()
	{
		unsigned char status;
		status = _device.Read(STATUS);
		_device.RW_Reg(WRITE_REG + STATUS, status);// clear RX_DR or TX_DS or MAX_RT interrupt flag
	}

	int Radio::_sendPayload(unsigned char * ptr, uint8_t nb)
	{

		boolean enviado = false;
		while (!enviado)
		{
			_clearStatus();
			_device.Write_Buf(WR_TX_PLOAD, ptr, nb);       // write playload to TX_FIFO
			unsigned char status;
			do
			{
				status = _device.Read(STATUS);
			} while (!(status&TX_DS) && !(status&MAX_RT));

			if (!(status&MAX_RT))
			{
				enviado = true;
			}

		}
		return nb;
	}

	void Radio::Write(void * _buf, uint32_t tam)
	{
		unsigned int units = tam / TX_UNIT;
		unsigned int left = tam % TX_UNIT;
		uint8_t * ptr = (uint8_t *)_buf;
		uint8_t * maxptr = ptr + (TX_UNIT)*units;

		while (ptr != maxptr)
		{
			int bytes = _sendPayload(ptr, TX_UNIT);
			ptr += bytes;
		}
		if (left>0)
		{
			int bytes = _sendPayload(ptr, left);
			ptr += bytes;
		}
	}

}