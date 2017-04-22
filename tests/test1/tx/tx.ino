#include "Radio.h"
#include <stdint.h>
#include "tarjeta.h"
#include <LCD5110_Graph.h>

#define BUFF_SIZE 200
using namespace ami;

uint8_t buff[BUFF_SIZE];
char msg[BUFF_SIZE];
unsigned long time, lastTime;

Radio radio;

LCD5110 display(D_SCK, D_MOSI, D_DC, D_RES, D_CS);
extern uint8_t SmallFont[], TinyFont[];

void initShield()
{
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

void updateDisplay(bool force = false)
{
	time = millis() / 1000;
	if (force || time != lastTime)
	{
		lastTime = time;
		sprintf(msg, "Current second: %d", time);
		display.clrScr();
		display.print(msg, CENTER, 0);
		sprintf(msg, "TX: %s", buff);
		display.print(msg, CENTER, 16);
		display.update();
	}
}

void setup()
{
	/* add setup code here */
	Serial.begin(115200);
	radio.Init(false,
		RF_CS,
		RF_CSN,
		RF_SCK,
		RF_MOSI,
		RF_MISO,
		RF_IRQ);
	radio.SetChannel(40);
	
	radio.SetRxAddr(1);
	radio.SetTxAddr(2);
	radio.TxMode();

	initShield();
}

void loop()
{
	int n;

	updateDisplay();
	if ((n = Serial.available()) > 0)
	{
		Serial.readBytes(buff, n);
		buff[n] = 0;
		Serial.print("Enviando: ");
		Serial.write(buff, n);
		Serial.println();
		radio.Write(buff, n);
		updateDisplay(true);
	}


}
