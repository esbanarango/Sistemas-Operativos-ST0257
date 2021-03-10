##Práctica 1, Sitemas Operativos

###We Fork We Execute

 **Por:**
  
   * [Esteban Arango Medina](https://github.com/esbanarango)
   * [Daniel Duque Tirado](https://github.com/dduqueti)


Introducción
==
Los sistemas de control de trabajos [Job control](http://en.wikipedia. org/wiki/Job_control), permite la gestión de múltiples trabajos de control teniendo como objetivo el adecuado manejo de recursos para impedir situaciones indesables como bloqueos mutuos (deadlocks).
Existen diferentes maneras diseñar e implementar los sistemas de control, uno de los más reconocidos es de lenguaje de control de trabajos (Job control language JCL). En este sistema se asume que los procesos llevan un vida ordenada: nacen, procesan y mueren, en una única vida.
En esta práctica vamos a cambiar el modelo de vida de los procesos y le vamos a permitir que los procesos vuelvan a vida según un número de veces predefinidas.

###Modelo del sistema de control
En este sistema de control cambiaremos el modelo de manejo de procesos de los sitemas de control tradicionales. El proceso llamado consola de control es el proceso principal que se encarga de iniciar todo el sistema de control. La configuración de arranque del sistema de control es obtenida al leer el archivo de configuración que indica, la cantidad y permanencia de los procesos suicidas que serán controlados.
Una vez leído el archivo de configuración e identificado cada programa a controlar, se iniciará un hilo de consola por cada proceso a controlar; este a su vez se encargará de crear un proceso controlador, indicandole explícitamente cuál es el proceso suicida a controlar y por cuántas veces debe mantenerlo vivo.

###Componentes
#####Archivo de configuración
El archivo de configuración describe la información pertinente al arranque del sistema. La sintaxis del archivo de configuración es la siguiente:

	ArchCfg 	→ ε
			    | ProcesoSui ArchCfg
	ProcesoSui  → ’ProcesoSui’Id’{’Path’::’Filename NúmeroVidas’}’

Cada proceso es identificado de forma única con Id (un identificador similar a los Java); Path1 identifica la ruta donde estú ubicado el ejecutable Filename). La estabilidad de cada proceso es identificada a través de Número entero positivo donde 0 significa que vive por siempre y n > 0 es el valor del número de veces que debe vivir.

El siguiente es el contenido de un posible archivo de configuración:
		
	ProcesoSui primerSuicida { /home/fcardona/bin :: ProcesoSuicida 10 } 
	ProcesoSui eternoSuicida { /usr/local/bin :: TendenciasSuicidas 0 }

El proceso identificado con primerSuicida está ubicado en el directorio `/home/fcardona/bin` y el nombre del archivo es _ProcesoSuicida_ y se ejecutará 10 veces. El segundo proceso suicida identificado con eternoSuicida está ubicado en el directorio `/usr/local/bin` y tiene como nombre de archivo _TendenciasSuicidas_ y lo intentará eternamente.

#####Consola de control
Es el encargado de leer el archivo de configuración interpretarlo y crear el sistema de control completo con respecto al archivo de configuración. El nombre del ejecutable es _conctrl_.

#####Hilos de consola
Son los encargados de comunicarse con los programas procesos controladores. Ellos se encargan de mostrar los mensajes enviados por los procesos controladores en la terminal donde la consola de control corre.
Cada hilo de consola se conecta directamente con un proceso controlador utilizando tuberías (pipes). El hilo de control se comunica por medio de tres tuberías que conectan con los respectivas entrada, salida y error estándar de este proceso.
El hilo de consola se comunica con el proceso de controlador por medio de la entrada estándar para enviar comandos. Recibe las respuestas del proceso controlador y las imprime en la terminal asociada a la consola de control, lo mismo sucede con los mensajes enviados por el error estándar. Todos los mensajes que son recibidos por parte del proceso controlador son inmediamente recibidos e impresos.

#####Proceso de control
El proceso de control se encarga de controlar un proceso suicida, cada vez que este proceso suicida termine por la razón que sea debe informar a su respectivo hilo de control la causa del desceso a través de la salida estándar:

![PUSH](https://github.com/esbanarango/Sistemas-Operativos-ST0257/raw/master/Pra%CC%81ctica%201/screenshotExample.png)

Luego de informar debe reiniciar el proceso suicida decrementando el valor de vidas restantes que le queda hasta que llegue a cero, en cuyo caso, el proceso de control de termina, informando que su labor fue llevada a cabo.
Cada proceso de control está identificado con un número único (además del PID).
Para controlar los procesos suicidas los procesos de control están conectados completamente a ellos a través de tuberías.
Los procesos de control tiene la siguiente línea de comandos para iniciar:


	procesoctrl --id=<id del proceso>￼
				--filepath=<path del ejecutable>￼
				--filename=<nombre del ejecutable> 
				--reencarnacion=<número de reencarnaciones> 
				<número del proceso de control>


######Procesos suicidas
Los procesos son un grupo de programas que siempre que los inicien terminarán por las más diversas razones.

