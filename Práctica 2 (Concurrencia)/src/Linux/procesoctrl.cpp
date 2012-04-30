#include "libs.h"
#include <stdlib.h>

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
  int idMemoria,idSemaforoMemoria;
}process;

/********** Concurrencia **********/
typedef struct str_infoMuerte { 
    long seq; 
    int nDecesos;
}infoMuerte;

typedef struct str_memoriaCompartida {

    int n; // Número de procesos controladores
    long valSeq;
    infoMuerte muertes[254];// Cada entrada identifica la información
                               // de cada proceso suicida.
}memoriaCompartida;

/* Simple lock operation. 0=which-semaphore, -1=decrement, 0=noflags  */
struct sembuf lockOperation = { 0, -1, 0};
/* Simple unlock operation. 0=which-semaphore, 1=increment, 0=noflags */
struct sembuf unlockOperation = { 0, 1, 0};

process getParamsToProcess(int argc, char *argv[]);
string coloriar(int color, string texto);
void fCreateProcess(process *p);
void* readOut(void *ptr);
void* readErr(void *ptr);
string intToStr(int i);

//Dirección de la memoria compartida;
memoriaCompartida* direccionCompartida;
int idSemCtrlConcurrencia;

int main(int argc, char *argv[]){
    process p;
    p = getParamsToProcess(argc,argv);
	fCreateProcess(&p);
	return 0;
}

process getParamsToProcess(int argc, char *argv[]){

    process p;

    int c;
    int digit_optind = 0;

   while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"id",     required_argument, 0,  0 },
            {"filepath",     required_argument, 0,  0 },
            {"filename",  required_argument,       0,  0 },
            {"idMemoria",  required_argument,       0,  0 },
            {"idSemaforoMemoria",  required_argument,       0,  0 }, 
            {"reencarnacion",  required_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

       c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);
        if (c == -1)
            break;

       switch (c) {
        case 0:
            string argumento = long_options[option_index].name;
            if (argumento == "id"){
                p.id = optarg;
            }else if(argumento == "filepath"){
                p.path = optarg;
            }else if(argumento == "filename"){
                p.fileName = optarg;
            }else if(argumento == "reencarnacion"){
                p.lives = atoi(optarg);
            }else if(argumento == "idMemoria"){
                p.idMemoria = atoi(optarg);
            }else if(argumento == "idSemaforoMemoria"){
                p.idSemaforoMemoria = atoi(optarg);
            }
            break;
        }
    }

   if (optind < argc) {
        p.control = atoi(argv[optind++]);
    }
    return p;
}


void fCreateProcess(process *p){

    int idSeg;
    memoriaCompartida *mComp;
    int idSem;
    int idSegSemCtrlConcurrencia;

    if ((idSeg = shmget(p->idMemoria, PAGE_SIZE, 0)) < 0) {
        fprintf(stderr, "Fallo al crear el segmento de memoria debido a: %d %s\n",
            errno, strerror(errno));
        exit(1);
      }

    if ((mComp = (memoriaCompartida *) shmat(idSeg, 0, 0)) == (void *) 0) {
        fprintf(stderr, "No pudo ser asignado el segmento de memoria: %d %s\n",
            errno, strerror(errno));
        exit(1);
      }
    if ((idSem = semget(p->idSemaforoMemoria,  1, IPC_CREAT | 0666)) < 0) {
        fprintf(stderr, "No se pudo obtener semaforo excl: %d %s\n",
        errno, strerror(errno));
        exit(1);
    }

    /* Crear semaforo para control de concurrencia de procesos hijo */
    if ((idSemCtrlConcurrencia = semget(idSegSemCtrlConcurrencia, 1, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        fprintf(stderr, "No se pudo crear un semaforo: %d %s\n",
        errno, strerror(errno));
        exit(1);
    }

    if (semctl(idSemCtrlConcurrencia, 0, SETVAL, (int)1) < 0) {
        fprintf(stderr, "No se pudo establecer el valor del semaforo: %d %s\n",
            errno, strerror(errno));
        exit(1);
      }

    do{
    	int causa;
    	int retVal;

        //Pipes
        int in[2];
        int out[2];
        int err[2];
        
        pid_t pid;
        
        pipe(in);
        pipe(out);
        pipe(err);

        if ( (pid = fork()) < 0 )
        {
            cerr << "FORK ERROR" << endl;
            exit(0);
        }
        else  if (pid == 0)     // CHILD PROCESS
        {
            close(in[1]);
            close(out[0]);

            dup2(in[0], STDIN_FILENO);
            close(in[0]);

            dup2(out[1], STDOUT_FILENO);
            close(out[1]);

            dup2(err[1], STDERR_FILENO);
            close(err[1]);

            execl(p->path.c_str(), p->fileName.c_str(),(char *) 0);
            // No se debe ejecutar este código
            fprintf(stderr, "No pudo ejecutar procesoctrl %s\n", strerror(errno));
        }
        else        // PARENT PROCESS
        {
            int rv;
            /* NOTA: Dado que el bufer es limitado, puede llenarse y puede combinarse con algún valor
            anterior. Esto no implica que las salidas se estén entreponiendo. Para esto se incorporan
            semáforos en sección crítica de impresión */
            close(in[0]);
            close(out[1]);
            close(err[1]);

            pthread_t readingIn;
            pthread_t readingErr;


            pthread_create(&readingIn, NULL, &readOut, (void *) &out[0]);
            pthread_join(readingIn,NULL);
            pthread_create(&readingErr, NULL, &readErr, (void *) &err[0]);
            pthread_join(readingErr,NULL);
            
            wait(&retVal);

            // Verifica si el hijo terminó bien
            if (WIFEXITED(retVal)) {
                causa = WEXITSTATUS(retVal);
            }
            else if (WIFSIGNALED(retVal)) { // Fue señalizado
                causa = WTERMSIG(retVal);
            }
            else if (WIFSTOPPED(retVal)) {
                causa = WSTOPSIG(retVal);
            }

            /*** SECCIÓN CRÍTICA***/
            if (semop(idSem, &lockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible senalar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            mComp->valSeq++;
        
            mComp->muertes[p->control-1].seq= mComp->valSeq;
            mComp->muertes[p->control-1].nDecesos++;

            //Realizar operación en semáforo
            if (semop(idSem, &unlockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible soltar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            /*** FIN SECCIÓN CRÍTICA***/

            /*** SECCIÓN CRÍTICA ESCRITURA***/
            if (semop(idSemCtrlConcurrencia, &lockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible senalar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            cout<<"Proceso suicida "<<coloriar(VERDE,p->id)<<" termino por causa ";
            cout<<coloriar(ROJO,intToStr(causa))<<" -- Proceso Control "<<coloriar(AMARILLO,intToStr(p->control))<<", vidas restantes: "<<((--p->lives<0)?"infinitas":intToStr(p->lives))<<endl;

            //Realizar operación en semáforo
            if (semop(idSemCtrlConcurrencia, &unlockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible soltar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            /*** FIN SECCIÓN CRÍTICA ESCRITURA***/
            

         }

    }while(p->lives != 0);
    semctl(idSemCtrlConcurrencia,1,IPC_RMID);
}

void* readOut(void *ptr){
    int *in = (int*) ptr;
    int rv=1;
    char line[MAXLINE];

    while(rv>0){
        fflush(stdin);
        rv = read(*in, line, MAXLINE);
        if(rv>0){    

            /*** SECCIÓN CRÍTICA ESCRITURA***/
            if (semop(idSemCtrlConcurrencia, &lockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible senalar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            write(1,line,rv);
            //Realizar operación en semáforo
            if (semop(idSemCtrlConcurrencia, &unlockOperation, 1) < 0) {
                fprintf(stderr, "No fue posible soltar el semaforo: %d %s\n",
                errno, strerror(errno));
                exit(1);
            }
            /*** FIN SECCIÓN CRÍTICA ESCRITURA***/



           
        }
    }
}

void* readErr(void *ptr){
    int *in = (int*) ptr;
    int rv=1;

    char line[MAXLINE];

    while(rv>0){
        char line[MAXLINE];
        rv = read(*in, line, MAXLINE);
        if(rv>0){
            write(2,line,rv);            
            //cerr<<line;
        }
    }
}

string coloriar(int color, string texto){
    string resp=""; 
    switch(color){
        case VERDE:
            resp="\e[1m\e[32m";
        break;

        case ROJO:
            resp="\e[1m\e[31m";
        break;

        case AMARILLO:
            resp="\e[1m\e[33m";
        break;
    }
    return resp+=texto+"\e[0m";
       
}

 string intToStr(int i){
      string s;
      stringstream out;
      out << i;
      s = out.str();
      return s;
}