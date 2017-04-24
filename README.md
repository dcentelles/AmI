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
El directorio "examples" contiene los siguientes programas:
* _simple_link_: contiene dos firmwares (transmisor y receptor) para establecer un enlace half-duplex entre dos Arduino.
* _star_: implementa un protocolo para comunicar un número limitado de motas en una red con topología de estrella. La red se constituye de un nodo master y todos los demás esclavos. Al arrancar el firmware se le pide al usuario que establezca el tipo de nodo: master o esclavo, y su configuración. En caso de ser un esclavo, se le pide al usuario su identificador. Por otro lado, si se trata del master es necesario especificar el número de nodos máximo de la red.

Estos ejemplos pueden servir como plantilla o referencia para desarrollar protocolos para redes de sensores inalámbricos.
