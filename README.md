# AmI
Repositorio para la asignatura de AmI

## Dependencias
Última versión de Arduino IDE. Se ha testeado con la v1.8.2.

Instalar las librerías "LCD5110_Graph", "StandardCplusplus", "TimerOne-r11" disponibles en este enlace: 
https://www.dropbox.com/sh/6owtwqxmz0l5fe8/AACHLIbu8VwnJv-0Efo25IORa?dl=0

Y las dos que hay en el directorio "libraries" de este repositorio ("AmlShield" y "Radio").

A continuación, una breve explicación de cada una de estas dependencias:

* **LCD5110_Graph** es una versión ligeramente modificada de la librería de la LCD para que compile con la última versión de Arduino IDE.
* **StandardCplusplus** es una versión ligera de algunas utilidades de la librería stl de c++, para poder usar std::list, std::vector, etc. en Arduino.
* **TimerOne-r11** es para poder programar fácilmente con Interrupcioes en Arduino. Documentación en:
	* [Arduino y los timers](http://www.prometec.net/timers/).
	* [TimerOne en la web de Arduino](http://playground.arduino.cc/Code/Timer1).
* **AmlShield** es para utilizar la shield de Arduino que usamos en la asignatura. 
* La librería que tendremos que extender (seguramente) es **Radio**. O bien crear una nueva sobre ella... 

## Ejemplos

En el directorio "tests" contiene programas de ejemplos que nos pueden servir como plantilla para desarrollar el protocolo pensado en la asignatura de AmI.
