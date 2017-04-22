#ifndef RADIO
#define RADIO

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

		void Init(bool autoAck = false,
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
		void BroadcastMode(bool);
		void SetRxAddr(const uint8_t *);
		void SetRxAddr(uint8_t ad0, uint8_t ad1 = 0xff, uint8_t ad2 = 0xff, uint8_t ad3 = 0xff, uint8_t ad4 = 0xff);
		void SetTxAddr(const uint8_t *);
		void SetTxAddr(uint8_t ad0, uint8_t ad1 = 0xff, uint8_t ad2 = 0xff, uint8_t ad3 = 0xff, uint8_t ad4 = 0xff);
		int Available();

	private:
		nRF24L01 _device;
		void _init_io();
		void _clearStatus();
		int _readyForRead();
		int _sendPayload(unsigned char * ptr, uint8_t nb);
		int _getRxPldWidth();
		void _setTxAddr(const uint8_t * addr);
		void _setRxAddr();

		uint8_t BRD_ADDR[ADR_WIDTH] =
		{
			0xff,0xff,0xff,0xff,0xff
		};

		uint8_t ADR[ADR_WIDTH] =
		{
			1,1,1,1,1
		};
		uint8_t TX_ADR[ADR_WIDTH];

		uint8_t _buff[TX_UNIT + 1];
		uint8_t _available = 0;
		uint8_t _status;
		uint8_t _fifostatus;
		bool _autoAck;
	};
}

#endif