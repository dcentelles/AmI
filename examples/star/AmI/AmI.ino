#include "Radio.h"
#include <stdint.h>
#include "tarjeta.h"
#include <LCD5110_Graph.h>

#define IZQ		1		// Valores para la salida de la funcion de teclado
#define DER		2		// Indican pusador derecho, izquierdo o ambos
#define AMBOS	3		// pulsados

#define BUFF_SIZE 36

#define NFAMILIES 8
#define MAX_SLAVES_PER_FAMILY 31

using namespace ami;

uint8_t buff[BUFF_SIZE];
char msg[BUFF_SIZE];
int second, lastSecond, family, lastFamily, 
lastSyncSecond; //slave
unsigned long lastSyncMillis; //slave
int ownFamily; //slave

Radio radio;

LCD5110 display(D_SCK, D_MOSI, D_DC, D_RES, D_CS);
extern uint8_t SmallFont[], TinyFont[];
uint8_t rxAddr, txAddr;
bool master;
bool sincronizado, firstIteration;
uint8_t nslaves;

void initShield()
{
	// Inicializacion del display, con tipo de letra y luz
	display.InitLCD();
	display.setFont(TinyFont);
	//display.setFont(SmallFont);

	// Inicializacion general de los pines
	pinMode(PULS1, INPUT);
	pinMode(PULS2, INPUT);
	pinMode(LEDR, OUTPUT);
	pinMode(LEDV, OUTPUT);
	pinMode(D_LED, OUTPUT);
	digitalWrite(PULS1, HIGH);
	digitalWrite(PULS2, HIGH);
	pinMode(D_LED, OUTPUT);

 digitalWrite(D_LED, LOW);
}

// Pasamos de 4 bits a una cifra hexadecimal.
inline char ByteAHexa(byte a)
{
	return (((a)<10) ? (a + '0') : (a)+'A' - 10);
}

// Sencilla funcion de consulta de los pulsadores. Devuelve 0 si no hay
// nada pulsado o IZQ, DER o AMBOS segun lo que haya pulsado. No espera
// ni bloquea.
int teclado()
{
	int salida = 0;

	if (!digitalRead(PULS1))
		salida += 1;
	if (!digitalRead(PULS2))
		salida += 2;

	return(salida);
}

void requestConf()
{
	rxAddr = 0;
	master = false;

	char addrMsg[20] = "ADDR: 0x    ";
	char msMsg[20] = "Master/Slave: S  ";

	int selAddrPos = strlen(addrMsg) - 1;
	int selMsPos = strlen(msMsg) - 1;

	int msPos = selMsPos - 2;

	int d1pos = selAddrPos - 3;
	int d0pos = selAddrPos - 2;

	addrMsg[d1pos] = ByteAHexa(rxAddr >> 4);
	addrMsg[d0pos] = ByteAHexa(rxAddr & 0xf);

	addrMsg[selAddrPos] = '<';

	display.clrScr();
	display.print("Configuration:", CENTER, 0);
	display.print(msMsg, CENTER, 16);
	display.update();

	int pulsado;

	//Pedimos si es master (inicia comms.) o esclavo (espera)
	while (1)
	{
		display.print(msMsg, CENTER, 16);
		display.update();
		delay(100);

		pulsado = teclado();
		if (pulsado == DER || pulsado == IZQ) {
			if (master)
			{
				master = false;
				msMsg[msPos] = 'S';
			}
			else
			{
				master = true;
				msMsg[msPos] = 'M';
			}
		}
		else if (pulsado == AMBOS) {
			break;
		}
	}
	msMsg[selMsPos] = ' ';
	display.print(msMsg, CENTER, 16);
	display.update();
	delay(500);

	int maxSlaves = MAX_SLAVES_PER_FAMILY * NFAMILIES;
	if (master)
	{
		rxAddr = 254;
		//Pedimos el n�mero de esclavos
		nslaves = 1;
		char nslavesMsg[20] = "Num. Slaves: 0x   <";
		int selSlavesPos = strlen(nslavesMsg) - 1;
		int ns1 = selSlavesPos - 3;
		int ns0 = selSlavesPos - 2;
		nslavesMsg[ns1] = ByteAHexa(nslaves >> 4);
		nslavesMsg[ns0] = ByteAHexa(nslaves & 0xf);

		while (1)
		{
			display.print(nslavesMsg, CENTER, 30);
			display.update();
			delay(100);

			pulsado = teclado();
			if (pulsado == DER) {
				nslaves++;
				if (nslaves > maxSlaves) nslaves = 1;
			}
			else if (pulsado == IZQ) {
				nslaves--;
				if (nslaves == 0) nslaves = maxSlaves;
			}
			else if (pulsado == AMBOS) {
				break;
			}
			nslavesMsg[ns1] = ByteAHexa(nslaves >> 4);
			nslavesMsg[ns0] = ByteAHexa(nslaves & 0xf);
		}
		initMaster();
	}
	else
	{
		//Pedimos la direccion de esclavo
		while (1)
		{
			display.print(addrMsg, CENTER, 30);
			display.update();
			delay(100);

			pulsado = teclado();
			if (pulsado == DER) {
				rxAddr++;
				if (rxAddr >= maxSlaves) rxAddr = 0;
			}
			else if (pulsado == IZQ) {
				rxAddr--;
				if (rxAddr >= maxSlaves) rxAddr = maxSlaves -1;
			}
			else if (pulsado == AMBOS) {
				break;
			}
			
			addrMsg[d1pos] = ByteAHexa(rxAddr >> 4);
			addrMsg[d0pos] = ByteAHexa(rxAddr & 0xf);
		}
		initSlave();

	}
}

class Mensaje
{
public:
	enum Tipo {
		T1, //Sincronizacion
		T2, //Peticion master a esclavo 
		T3  //Respuesta de esclavo
	};

	const static int 
		T1L = 4,
		T2L = 2,
		T3L = 22,
		T3PL = 20;
	static bool GetNextMessage(Radio & radio, Mensaje & msg)
	{
		int n = radio.Available();
		if (n > 0)
		{
			Serial.print("Available: "); Serial.println(n);
			uint8_t * ptr = msg.Buffer;
			radio.Read(ptr, 1); 
			Tipo tipo = (Tipo) *ptr;
			ptr++;
			switch (tipo)
			{
			case T1:
				Serial.print("Leyendo: "); Serial.println(T1L);
				radio.Read(ptr, T1L);
				Serial.println("Recibido T1");
				break;
			case T2:
				Serial.print("Leyendo: "); Serial.println(T2L);
				radio.Read(ptr, T2L);
				Serial.println("Recibido T2");
				break;
			case T3:
				Serial.print("Leyendo: "); Serial.println(T3L);
				radio.Read(ptr, T3L);
				Serial.println("Recibido T3");
				break;
			default:
				Serial.println("FATAL ERROR: unknown type");
				return false;
			}
			return true;
		}
		return false;
	}

	Mensaje()
	{
		_tipo = Buffer;
		_trAddr = _tipo + 1;
		_reAddr = _trAddr + 1;
		_tiempo = (uint16_t *)(_reAddr + 1);
		_datos = (uint8_t*)_tiempo;
	}

	Tipo GetTipo()
	{
		return (Tipo)*_tipo;
	}

	void SetTr(uint8_t tr)
	{
		*_trAddr = tr;
	}

	uint8_t GetTr()
	{
		return *_trAddr;
	}
	void SetRe(uint8_t re)
	{
		*_reAddr = re;
	}

	uint8_t GetRe()
	{
		return *_reAddr;
	}
	void SetTipo(Tipo tipo)
	{
		*_tipo = tipo;
	}

	uint16_t GetTiempo()
	{
		Serial.println(*_tiempo);
		return *_tiempo;
	}

	uint16_t SetTiempo(uint16_t tiempo)
	{
		*_tiempo = tiempo;
	}

	uint8_t Length()
	{
		uint8_t res = 0;
		switch (GetTipo())
		{
		case T1:
			res = T1L;
			break;
		case T2:
			res = T2L;
			break;
		case T3:
			res = T3L;
			break;
		default:
			return 0;
		}
		res += 1;
		return res;
	}
	uint8_t Buffer[BUFF_SIZE];
private:
	uint8_t * _tipo;
	uint8_t * _trAddr, *_reAddr;
	uint16_t * _tiempo;
	uint8_t * _datos;
};


Mensaje mensaje;

struct CommsState
{
  uint8_t slaves[NFAMILIES][MAX_SLAVES_PER_FAMILY];
  uint8_t nSlaves[NFAMILIES];
  uint8_t nextSlave[NFAMILIES];
};

CommsState commsState;
void initMaster()
{
	Serial.print("1 Nslaves: "); Serial.println(nslaves);
	for (int f = 0; f < NFAMILIES; f++)
	{
		commsState.nSlaves[f] = 0;
		commsState.nextSlave[f] = 0;
	}

	Serial.print("2 Nslaves: "); Serial.println(nslaves);

	for (int s = 0; s < nslaves; s++)
	{
		int sfamily = s & 0x7;
		int nSlaves = commsState.nSlaves[sfamily];
		commsState.slaves[sfamily][nSlaves] = s;
		Serial.print("slave: "); Serial.print(commsState.slaves[sfamily][nSlaves]); Serial.print("; family: "); Serial.println(sfamily);
		commsState.nSlaves[sfamily] += 1;
	}
	Serial.print("3 Nslaves: "); Serial.println(nslaves);
	display.clrScr();
	radio.SetRxAddr(rxAddr);
}

void initSlave()
{
	sincronizado = false;
	radio.SetRxAddr(rxAddr);
	ownFamily = rxAddr & 0x7;
	radio.RxMode();
	firstIteration = true;
}

void setup()
{
	Serial.begin(115200);
	radio.Init(false,
		RF_CS,
		RF_CSN,
		RF_SCK,
		RF_MOSI,
		RF_MISO,
		RF_IRQ);
	radio.SetChannel(40);

	initShield();
	requestConf();
}
void updateSlaveStateAndDisplay()
{
	updateSlaveState();
	updateSlaveStateOnDisplay();
}

void updateSlaveState()
{
	lastSecond = second;
	unsigned long _elapsedMillis = millis() - lastSyncMillis;

	second = (lastSyncSecond + (_elapsedMillis / 1000)) % 60;
	family = second / 6;
}
void updateSecondOnSlave(uint16_t _second)
{
	lastSyncSecond = _second;
	lastSyncMillis = millis();
	updateSlaveStateAndDisplay();
}

void updateSlaveStateOnDisplay()
{
	display.clrScr();
	sprintf(msg, "SLAVE (ADDR: %d)", rxAddr);
	display.print(msg, CENTER, 0);
	sprintf(msg, "Current second: %d", second);
	display.print(msg, CENTER, 10);
	sprintf(msg, "Current family: %d", family);
	display.print(msg, CENTER, 20);
	display.print((char*)buff, CENTER, 30);
	display.update();
}

void updateMasterStateOnDisplay()
{
	display.clrScr();
	sprintf(msg, "MASTER (NODES: %d)", nslaves);
	display.print(msg, CENTER, 0);
	sprintf(msg, "Current second: %d", second);
	display.print(msg, CENTER, 10);
	sprintf(msg, "Current family: %d", family);
	display.print(msg, CENTER, 20);
	display.print((char*)buff, CENTER, 30);
	display.update();
}

void updateMasterStateAndDisplay()
{
	updateMasterState();
	updateMasterStateOnDisplay();
}

void updateMasterState()
{
	int _lastSecond = second;
	second = (millis() / 1000) % 60;
	family = second / 6;
}

void MasterWork()
{
	updateMasterState();
	unsigned long _lastTime;
	unsigned long _elapsed;
	if (second != lastSecond) {
		updateMasterStateOnDisplay();
		lastSecond = second;
		if (family != lastFamily)
		{
			lastFamily = family;
			if(family < 8) //Pedimos datos a un esclavo
			{
				digitalWrite(LEDV, LOW);

				//Esperamos un tiempo para que se despierte el esclavo
				//TODO: tal cual est� ahora, el esclavo no apaga la radio nunca, pero estar�a bien para ahorrar bater�a...
				_lastTime = millis();
				_elapsed = 0;
				while (_elapsed < 500)
				{
					delay(10);
					updateMasterStateAndDisplay();
					_elapsed = millis() - _lastTime;
				}

				//Obtenemos direccion del siguiente esclavo y lo situamos al final de la cola
				int nSlaves = commsState.nSlaves[family];
				
				if (nSlaves > 0)
				{
					int nextSlave = commsState.nextSlave[family];
					txAddr = commsState.slaves[family][nextSlave];
					nextSlave = (nextSlave + 1) % nSlaves;
					commsState.nextSlave[family] = nextSlave;

					//Ponemos la radio modo transmision
					radio.SetTxAddr(txAddr);
					radio.TxMode();

					//Esperamos a que el esclavo est� en modo escucha
					sprintf((char*)buff, "Esperando a: %d", txAddr);
					updateMasterStateAndDisplay();
					_lastTime = millis();
					_elapsed = 0;
					while (_elapsed < 1000)
					{
						delay(10);
						updateMasterStateAndDisplay();
						_elapsed = millis() - _lastTime;
					}

					//Enviamos peticion de datos al esclavo
					mensaje.SetTipo(Mensaje::T2);
					mensaje.SetTr(rxAddr);
					mensaje.SetRe(txAddr);
					//mostrarEnvioT2();
					radio.Write(mensaje.Buffer, mensaje.Length());
					sprintf((char*)buff, "Enviado T2");
					updateMasterStateAndDisplay();
					radio.RxMode();
					//Esperamos respuesta del esclavo
					_lastTime = millis();
					_elapsed = 0;
					bool received = false;
					while (_elapsed < 2500 && !received)
					{
						received = Mensaje::GetNextMessage(radio, mensaje);
						if (received)
						{
							if (mensaje.GetTipo() == Mensaje::T3 && mensaje.GetRe() == rxAddr)
							{
								if (mensaje.GetTr()!= txAddr)
								{
									sprintf((char*)buff, "Error origen T3");
									received = false;
								}
							}
							else
							{
								sprintf((char*)buff, "Error Tipo mensaje");
								received = false;
							}
						}
						updateMasterStateAndDisplay();
						_elapsed = millis() - _lastTime;
					}
					if (received)
					{
						//mostrarRecepcionT3();
						digitalWrite(LEDR, LOW);
						sprintf((char*)buff, "Recibido T3");
						//TODO: aqu� faltar�a procesar los datos contenidos en el mensaje T3
					}
					else
					{
						digitalWrite(LEDR, HIGH);
						sprintf((char*)buff, "T3 Timeout");
					}
					updateMasterStateAndDisplay();
				}
			}
			else if (family == 8)
			{
				_lastTime = millis();
				_elapsed = 0;
				//Esperamos un tiempo para que se despierten los esclavos
				//TODO: tal cual est� ahora, el esclavo no apaga la radio nunca, pero estar�a bien para ahorrar bater�a...
				while (_elapsed < 500)
				{
					delay(10);
					updateMasterStateAndDisplay();
					_elapsed = millis() - _lastTime;
				}
				digitalWrite(LEDV, HIGH);
				//Enviamos sincronizamos todos los esclavos (broadcast)
				mensaje.SetTipo(Mensaje::T1);
				mensaje.SetTr(255);
				radio.SetTxAddr(255);
				mensaje.SetRe(txAddr);
				radio.TxMode();
				sprintf((char*)buff, "Sincronizando Eclavos");
				updateMasterStateOnDisplay();
				//Comprobamos segundo actual
				updateMasterState();
				int _currentSecond = second;
				while (_currentSecond == second) {
					updateMasterState();
				}
				mensaje.SetTiempo(second);
				radio.Write(mensaje.Buffer, mensaje.Length());
				updateMasterStateAndDisplay();
				radio.RxMode();
			}
			else {
				digitalWrite(LEDV, LOW);
			}
		}
	}
}

/*
void parpadeo(uint8_t pin, int ms, int iter)
{
	uint8_t pinValue = bitRead(PORTD, pin);
	uint8_t i = 0;
	while (i < iter)
	{
		digitalWrite(pin, !pinValue);
		delay(ms);
		digitalWrite(pin, pinValue);
		delay(ms);
		i++;
	}
}

void mostrarRecepcionT2()
{
	parpadeo(LEDR, 25, 5);
}

void mostrarEnvioT2()
{
	parpadeo(LEDR, 25, 5);
}

void mostrarRecepcionT3()
{
	parpadeo(LEDR, 25, 5);
}
*/

void SlaveWork()
{
	radio.RxMode();
	updateSlaveState();
	bool received;
	unsigned long _lastTime;
	unsigned long _elapsed;
	if (second != lastSecond)
	{
		updateSlaveStateAndDisplay();
		if (family == ownFamily && sincronizado && !firstIteration)
		{
			//TODO: Encender la radio (radio.On()) (por ahora esta siempre encendida, pero debe apagarse cuando no se usa)
			sprintf((char*)buff, "Wait Master Req.");
			updateSlaveStateAndDisplay();
			sincronizado = false; 

			//Esperamos T2
			radio.RxMode();
			_lastTime = millis();
			_elapsed = 0;
			received = false;
			while (_elapsed < 2000)
			{
				received = Mensaje::GetNextMessage(radio, mensaje);
				if (received)
				{
					if (mensaje.GetTipo() == Mensaje::T2 && mensaje.GetRe() == rxAddr)
					{
						received = true;
						break;
					}
				}
				delay(10);
				updateSlaveStateAndDisplay();
				_elapsed = millis() - _lastTime;
			}
			if (received)
			{
				//mostrarRecepcionT2();
				digitalWrite(LEDR, LOW);
				sprintf((char*)buff, "Recibido T2");
				updateSlaveStateAndDisplay();
				//Enviamos T3
				radio.TxMode();
				mensaje.SetTr(rxAddr);
				txAddr = 254;//Destino master
				mensaje.SetRe(txAddr);
				mensaje.SetTipo(Mensaje::T3);
				radio.SetTxAddr(txAddr);
				
				//Esperamos un tiempo para asegurarnos que el Master esta en modo RX
				delay(250); 
				
				radio.Write(mensaje.Buffer, mensaje.Length());
				sprintf((char*)buff, "Enviado T3");
				updateSlaveStateAndDisplay();
			}
			else
			{
				digitalWrite(LEDR, HIGH);
			}
			updateSlaveStateAndDisplay();
			radio.RxMode();

		}
		else if (family == 8 && !sincronizado || firstIteration) //Esperar sincronizacion
		{
			//TODO: Encender la radio (por ahora esta siempre encendida, pero debe apagarse cuando no se usa)
			firstIteration = false;
			//Esperamos mensaje de sincronizacion
			radio.RxMode();
			sprintf((char*)buff, "Wait Sync. msg");
			updateSlaveStateAndDisplay();
			received = false;
			while (!sincronizado)
			{
				updateSlaveStateAndDisplay();
				received = Mensaje::GetNextMessage(radio, mensaje);
				if (received)
				{
					if (mensaje.GetTipo() == Mensaje::T1)
					{
						updateSecondOnSlave(mensaje.GetTiempo());
						sprintf((char*)buff, "Sync: %d", second);
						updateSlaveStateAndDisplay();
						Serial.print("Sincronizado: "); Serial.println(second);
						sincronizado = true;
						digitalWrite(LEDV, HIGH);
					}
					else {
						Serial.println("Se esparaba un T1");
						received = false;
					}
				}
				else
				{
					updateSlaveStateAndDisplay();
					if(family != 8)
					{
						digitalWrite(LEDR, HIGH);
						digitalWrite(LEDV, LOW);
					}
				}
			}
		}
		else if(!firstIteration && family != 8 && family != ownFamily)
		{
			sprintf((char*)buff, "Sleep");
			updateSlaveStateAndDisplay();
			//TODO: apagar radio: radio.Off();
			//TODO: dormir la CPU de la Arduino
		}
	}
}

void loop()
{
	if (master) while(1) MasterWork();
	else while(1) SlaveWork();
}
