#pragma once
#include <stdint.h>
#include "nRF24L01.h"

namespace ami
{

#define ADR_WIDTH    5
#define BUFFER_SIZE 1500
#define TX_UNIT 32

	class Radio
	{
	public:
		Radio();
		~Radio();
		void Write(void * _buf, uint32_t tam);

		void Init(uint8_t *addr, bool autoAck = false,
			int ce = 8,
			int csn = 9,
			int sck = 10,
			int mosi = 11,
			int miso = 12,
			int irq = 13);

		void TxMode();
		void RxMode();
		void Read(void * dst, unsigned int req); //Blocking call
		void SetChannel(int channel);
		void SetAddr(uint8_t *);
		void SetTxAddr(uint8_t *);
		int Available();

	private:
		nRF24L01 _device;
		void _init_io();
		void _clearStatus();
		int _readyForRead();
		int _sendPayload(unsigned char * ptr, uint8_t nb);
		int _getRxPldWidth();

		unsigned char BROADCAST_ADDRESS[ADR_WIDTH] =
		{
			0xff,0xff,0xff,0xff,0xff
		};

		uint8_t ADR[ADR_WIDTH];

		uint8_t _buff[TX_UNIT + 1];
		uint8_t _available = 0;
		uint8_t _status;
		uint8_t _fifostatus;
		bool _autoAck;
	};
}