 /* CrearPThread.c */

/* $Id: CrearPThread.c,v 1.1.1.1 2003/06/19 19:00:15 fcardona Exp $ */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void*
funcion_hilo(void *arg) {
  
  int valor = (int) arg;

  fprintf(stdout, "Hola Mundo del hilo %d\n", valor);

  sleep(valor);

  fprintf(stdout, "Va a terminar hilo %d\n", valor);

  valor *= 100;

  return (void *) valor;
}

int
main(int argc, char *argv[]) {

  int nHilos = 0;           
  int i;
  pthread_t *tablaDeHilos;  // Información de los hilos
  int valorRetorno;

  if (argc != 2) {
    fprintf(stderr, "Uso: %s nHilos\n", argv[0]);
    exit(1);
  }

  nHilos = atoi(argv[1]);

  if (nHilos == 0) {
    fprintf(stderr, "Uso: %s nHilos\n", argv[0]);
    exit(1);
  }

  // Solicito memoria dinámica para la tabla
  tablaDeHilos = (pthread_t *) malloc(sizeof(pthread_t) * nHilos);
  
  for (i = 0; i < nHilos; i++)
    pthread_create((tablaDeHilos + i), 
      NULL, 
      funcion_hilo,
      (void *) i);
  
  for (i = 0; i < nHilos; i++) {

    pthread_join(*(tablaDeHilos +i),
     (void **) &valorRetorno);
    fprintf(stdout, "Valor de retorno: %d del hilo: %d\n",
      valorRetorno, *(tablaDeHilos +i));
  }

  exit(1);
}
     
  
  