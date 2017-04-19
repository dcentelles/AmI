// tarjeta.h
//
// Definiciones para la tarjeta 8shield) 
// para el curso de Arduino.

// La tarjeta tiene dos pulsadores, un led
// rojo y uno verde, un zocalo para el modulo
// nRF24L01 y un display Nokia 5110

// Se conectara sobre una variante de 
// Arduino Uno

// Definiciones de pines 

// Pulsadores
#define PULS1	3
#define PULS2	4

// Leds
#define LEDR	5
#define LEDV	6

// nRF24L01
#define RF_MOSI	MOSI
#define	RF_MISO	MISO
#define RF_SCK	SCK
#define RF_IRQ	2	
#define RF_CS	8	
#define RF_CSN	7

// Display
#define D_MOSI	MOSI
#define D_SCK	SCK
#define D_LED	A4
#define D_CS	9
#define D_DC	10
#define D_RES	A5

// Interrupciones
#define INT_RF		0
#define INT_PULS	1
