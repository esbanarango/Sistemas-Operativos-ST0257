##Práctica 2

###We Fork We Execute

 **Por:**
  
   * [Esteban Arango Medina](https://github.com/esbanarango)
   * [Daniel Duque Tirado](https://github.com/dduqueti)


Práctica de concurrencia
==
La práctica está dividida en dos partes; manejo de la salida y el error estándar en la consola de control; y la gestión de estadísticas.

###Salida concurrente
El diseño de la consola de control tiene problemas, por que al utilizar hilos, estos comparten el envio de mensajes a la salida y el error estándar simúltaneamente, haciendo que en algunas ocasiones ocurra condiciones de concurso para mezclar la salida de dos o más hilos.
Esto debe evitarse utilizando semáforos, puesto que solamente un hilo puede escribir, ya sea en la salida estándar o en el error estándar.

###Estadísticas
El Gobierno Nacional y el DANE requieren de estadísticas de la mortandad de los procesos suicidas. Se requiere saber el número de veces exactas que el proceso suicida ha muerto en cualquier instante de tiempo.
Para lograr esto vamos a realizar varios cambios a nuestra implementación de la práctica 1.

1. Los encargados de llevar a cabo la recolección de la información estadística serán los procesos controladores.
2. Todos los procesos controladores compartirán una región de memoria donde se guardará la información estadística.
Esto implica que el proceso controlador se le añaden dos opciónes a la línea de comandos.

	      procesoctrl --id=<id del proceso>
	    		￼--filepath=<path del ejecutable>
	    		--filename=<nombre del ejecutable>
	    		--reencarnacion=<número de reencarnaciones>
	    		--idMemoria=IdentificadorMemoriaCompartido
	    		--idSemaforoMemoria=IdentificadorSemaforo
	    		<número del proceso de control>

	El **idMemoria** identifica la región de memoria donde que comparte todos los procesos controladores y **idSemaforoMemoria** es el identificador del semáforo que se encargará de controlar el acceso a la memoria compartida.

3. La memoria compartida tendrá la siguiente estructura de datos:

	    struct MemoriaCompartida {
	    	int n; // Número de procesos controladores
	    	long int valSeq;
	    	struct InfoMuerte muertes[254]; // Cada entrada identifica la información
	    				     // de cada proceso suicida.
	    };

4. La estructura donde se almacena la información de los decesos es la siguiente:

	    struct InfoMuerte { 
	    	long int seq; int nDecesos;
	    };

	El valor de **seq** se obtiene de incrementar por cada proceso de control el valor de **valSeq**. Cada vez que un proceso suicida muere cada proceso controlador debe incrementar el valor de **valSeq** y incrementar también el número de muertes de que lleva su suicida a cargo y registrar en el campo correspondiente la información actualizada.

##Solución

Para solucionar esta problematica de región crítica hicimos uso de las funciones de la biblioteca _sys/sem_ y _sys/shm_. Se generó una memoria compartida que fue utilizada por los procesos que contiene los datos presentados en la descripción del problema. 
Para evitar condiciones de carrera se planteó un semáforo para controlar la manipulación de esta memoria compartida.
Para controlar la sección de impresión entre hilos se utilizaron dos semáforos en el proceso _conctrl_, uno para
la salida estándar y otro para el error estándar. Además de dos semáforos por cada _procesoctrl_ (salida y error) para controlar
la escritura de los suicidas y la propia del _procesoctrl_. Así se asegura que cada _procesoctrl_ tendrá acceso exclusivo a la
salida estándar y al error estándar.

####Ejemplo.
Posible salida en la consola, teniendo el siguiente `ArchCfg.txt`:

	    ProcesoSui primerSuicida { Procesos/pSuicida :: pSuicida 10 }
	    ProcesoSui segundoSuicida { Procesos/pSuicida :: pSuicida 8 }
	    ProcesoSui tercerSuicida { Procesos/pSuicida :: pSuicida 5 }

![PUSH](https://github.com/esbanarango/Sistemas-Operativos-ST0257/blob/master/Pra%CC%81ctica%202%20\(Concurrencia\)/ScreenShotExample.png?raw=true)
