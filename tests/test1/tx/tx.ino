#include "Radio.h"
#include <stdint.h>
#include "tarjeta.h"
#include <LCD5110_Graph.h>

using namespace ami;
uint8_t buff[200];

Radio radio;
uint8_t addr[5]{ 1,1,1,1,2 };

LCD5110 display(D_SCK, D_MOSI, D_DC, D_RES, D_CS);
extern uint8_t SmallFont[], TinyFont[];

void setup()
{
	//uint8_t txaddr[5]{ 255,255,255,255,255 };
	uint8_t txaddr[5]{ 1,1,1,1,1 };
	/* add setup code here */
	Serial.begin(115200);
	radio.Init(addr, false,
		RF_CS,
		RF_CSN,
		RF_SCK,
		RF_MOSI,
		RF_MISO,
		RF_IRQ);
	radio.SetChannel(40);

	radio.SetTxAddr(txaddr);
	radio.TxMode();

	// Inicializacion del display, con tipo de letra y luz
	display.InitLCD();
	display.setFont(TinyFont);
	//display.setFont(SmallFont);
	digitalWrite(D_LED, HIGH);

	// Inicializacion general de los pines
	pinMode(PULS1, INPUT);
	pinMode(PULS2, INPUT);
	pinMode(LEDR, OUTPUT);
	pinMode(LEDV, OUTPUT);
	pinMode(D_LED, OUTPUT);
	digitalWrite(PULS1, HIGH);
	digitalWrite(PULS2, HIGH);
	pinMode(D_LED, OUTPUT);

}

char msg[100];

unsigned long time, lastTime;

void printCurrentTime()
{
	time = millis() / 1000;
	if (time != lastTime)
	{
		lastTime = time;
		sprintf(msg, "Current second: %d", time);
		display.clrScr();
		display.print(msg, CENTER, 0);
		display.update();
	}
}

void loop()
{
	int n;
	if ((n = Serial.available()) > 0)
	{
		Serial.readBytes(buff, n);
		Serial.print("Enviando: ");
		Serial.write(buff, n);
		Serial.println();
		radio.Write(buff, n);
	}


}
