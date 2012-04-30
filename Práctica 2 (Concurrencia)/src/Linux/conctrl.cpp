#include "libs.h"

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
  int idMemoria,idSemaforoMemoria;
  //Info concurrencia
  int idSem, indiceSemErr, indiceSemOut;
}process;


/**********	Concurrencia **********/
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

typedef struct str_ctrlConcurrencia {
	int *ty;
	int idSem, indiceSem;
	int control;
}ctrlConcurrencia;

//Functions
void verificarValores(memoriaCompartida *mComp);
string coloriar(int color, string texto);
process getInfoProcess(string s);
void printInfoProcess(process p);
void liberarMemoria(int idSeg);
bool checkValid(process p);
void* fConctrl(void *ptr);
void* readErr(void *ptr);
void* readOut(void *ptr);
struct sembuf fnlockOperation(int numeroSemaforo);
struct sembuf fnunlockOperation(int numeroSemaforo);
string intToStr(int i);


/* Simple lock operation. 0=which-semaphore, -1=decrement, 0=noflags  */
struct sembuf lockOperation;
/* Simple unlock operation. 0=which-semaphore, 1=increment, 0=noflags */
struct sembuf unlockOperation;


int main(){

	vector<process> processes; //Vector with all the information about the processes
	int nHilos = 0;           
    pthread_t *tablaDeHilos;  // Thread information
    memoriaCompartida *mComp;
    int idMemoria=292;
	int	idSemaforo=298;
	int idSeg;
    int idSegSem;


    //Read info from ArchCfg.txt
	string s;
	freopen("../Common/ArchCfg.txt", "r", stdin);
  	while (getline(cin,s)){
  		process p = getInfoProcess(s);
  		if (!checkValid(p)){
  			cout<<coloriar(ROJO,"Atención: ")<<"Porfavor revise el archivo 'ArchCfg.txt' este está mal formado."<<endl;
  			exit(1);
  		}
  		processes.push_back(p);
  		nHilos++;
	}
	//Print process information
	/*for (int i = 0; i < processes.size(); ++i)
	{
		printInfoProcess(processes[i]);
	}*/

	tablaDeHilos = (pthread_t *) malloc(sizeof(pthread_t) * nHilos);
	cout<<"Total hilos:"<<nHilos<<endl;

	//****************Creación de la memoría compartida.
	 
	

	//Creacion Memoria compartida
	if ((idSeg = shmget(idMemoria, PAGE_SIZE, IPC_CREAT | IPC_EXCL | 0660 )) < 0) {
	    fprintf(stderr, "Fallo al crear el segmento de memoria debido a: %d %s\n",
		    errno, strerror(errno));
	    exit(1);
	  }

	if ((mComp = (memoriaCompartida *) shmat(idSeg, 0, 0)) == (void *) 0) {
	    fprintf(stderr, "No pudo ser asignado el segmento de memoria: %d %s\n",
		    errno, strerror(errno));
	    exit(1);
	  }
	//Creación de semáforo para control de memoria compartida
	if ((idSegSem = semget(idSemaforo, 3, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
		fprintf(stderr, "No se pudo crear un semaforo: %d %s\n",
		errno, strerror(errno));
		exit(1);
	}

	if (semctl(idSegSem, 0, SETVAL, (int)1) < 0) {
	    fprintf(stderr, "No se pudo establecer el valor del semaforo: %d %s\n",
		    errno, strerror(errno));
	    exit(1);
	  }
	  if (semctl(idSegSem, 1, SETVAL, (int)1) < 0) {
	    fprintf(stderr, "No se pudo establecer el valor del semaforo: %d %s\n",
		    errno, strerror(errno));
	    exit(1);
	  }
	  if (semctl(idSegSem, 2, SETVAL, (int)1) < 0) {
	    fprintf(stderr, "No se pudo establecer el valor del semaforo: %d %s\n",
		    errno, strerror(errno));
	    exit(1);
	  }

	//Dirección de la memoria compartida e inicialización
	infoMuerte iniVeInf={0,0};
	//vector<infoMuerte> muertes(nHilos,iniVeInf);
	mComp->n=nHilos;
	mComp->valSeq=0;


	//Execute each process
	for (int i = 0; i < nHilos; i++){
		processes[i].control = i+1;
		processes[i].idMemoria = idMemoria;
		processes[i].idSemaforoMemoria = idSemaforo;
		//int idSem, indiceSemErr, indiceSemOut;
		processes[i].idSem=idSegSem;
		processes[i].indiceSemErr=2;
		processes[i].indiceSemOut=1;

		pthread_create((tablaDeHilos + i), NULL, &fConctrl, (void *) &processes[i]);
	}
	for (int i = 0; i < nHilos; i++) {
	    pthread_join(*(tablaDeHilos +i), NULL);
	}

	printf("\t\t¡Terminó!\n\n");

	verificarValores(mComp);


	//libero memoria de la estructura
	liberarMemoria(idSeg);
	semctl(idSegSem,1,IPC_RMID);

	return 0;
}

/******** HELPERS FUNCTIONS ********/

void* fConctrl(void *ptr){
	process *p;            
    p = (process *) ptr;  /* type cast to a pointer to process */
	/*
	procesoctrl --id=<id del proceso>
				--filepath=<path del ejecutable>
				--filename=<nombre del ejecutable> 
				--idMemoria=IdentificadorMemoriaCompartido
				--reencarnacion=<número de reencarnaciones>
				<número del proceso de control>
	*/
    string id = "--id="+p->id;
    string filepath="--filepath="+p->path;
    string filename="--filename="+p->fileName;
    string reencarnacion="--reencarnacion="+intToStr(p->lives);
    string idMemoria="--idMemoria="+intToStr(p->idMemoria);
    string idSemaforoMemoria="--idSemaforoMemoria="+intToStr(p->idSemaforoMemoria);
    string contrlNum=intToStr(p->control);

	pid_t pid;

	 //Pipes
    int in[2];
    int out[2];
    int err[2];
    
    pipe(in);
    pipe(out);
    pipe(err);

	if ( (pid = fork()) < 0 )
    {
        cerr << "FORK ERROR" << endl;
        exit(0);
    }
	if (pid == (pid_t)(-1)) {
	  fprintf(stderr, "%s, Fallo al hacer el fork\n",
	  strerror(errno));
	  exit(13);
	}
	else if (pid == 0) {// CHILD PROCESS

		close(in[1]);
        close(out[0]);

        dup2(in[0], STDIN_FILENO);
        close(in[0]);

        dup2(out[1], STDOUT_FILENO);
        close(out[1]);

        dup2(err[1], STDERR_FILENO);
        close(err[1]);

	  execl("procesoctrl", "procesoctrl", id.c_str(),filepath.c_str(),filename.c_str(),reencarnacion.c_str(),idMemoria.c_str(),idSemaforoMemoria.c_str(),contrlNum.c_str(), (char *) 0);
	  // No se debe ejecutar este código
	  fprintf(stderr, "No pudo ejecutar procesoctrl %s\n", strerror(errno));
	}
	else {// PARENT PROCESS

		int rv;
        close(in[0]);
        close(out[1]);
        close(err[1]);

        pthread_t readingIn;
        pthread_t readingErr;

        ctrlConcurrencia crtlIn={&out[0],p->idSem,p->indiceSemErr,p->control};
        ctrlConcurrencia crtlErr={&err[0],p->idSem,p->indiceSemOut,p->control};

        pthread_create(&readingIn, NULL, &readOut, (void *) &crtlIn);
        pthread_join(readingIn,NULL);
        pthread_create(&readingErr, NULL, &readErr, (void *) &crtlErr);
        pthread_join(readingErr,NULL);
    }
}


void verificarValores(memoriaCompartida *mComp){
	cout<<coloriar(AMARILLO,"-----------------------------------------------------")<<endl;
	cout<<coloriar(AMARILLO,"Verificación de los procesos controladores & suicidas:")<<endl;
	for (int i = 0; i < mComp->n; ++i)
	{
		cout<<coloriar(AMARILLO,"Proceso Control: ")<<coloriar(VERDE,intToStr(i+1))<<endl;
		cout<<"\t Terminó en la secuencia: "<<mComp->muertes[i].seq<<endl;
		cout<<"\t Número de decesos: "<<mComp->muertes[i].nDecesos<<endl;
	}

}

void liberarMemoria(int i){

	if (shmctl(i, IPC_RMID, NULL) < 0) {
		fprintf(stderr, "Fallo al borrar el segmento de memoria debido a: %d %s\n",
		errno, strerror(errno)); exit(1);
	}

}

 string intToStr(int i){
      string s;
      stringstream out;
      out << i;
      s = out.str();
      return s;
}


void* readOut(void *ptr){
	ctrlConcurrencia *ctrCon = (ctrlConcurrencia*) ptr;
	int rv=1;

	
	/* Semaforo 2, indice 1 */
	lockOperation = fnlockOperation(ctrCon->indiceSem);
	unlockOperation = fnunlockOperation(ctrCon->indiceSem);
	

	            while(rv>0){
	            	char line[MAXLINE];
					rv = read(*ctrCon->ty, line, MAXLINE);
					if(rv>0){
						/*** SECCIÓN CRÍTICA EN IMPRESIÓN (COUT/CERR)***/
			            if (semop(ctrCon->idSem, &lockOperation, 1) < 0) {
			                fprintf(stderr, "No fue posible senalar el semaforo: %d %s\n",
			                errno, strerror(errno));
			                exit(1);
			            }
			            write(1,line,rv);
						//Realizar operación en semáforo
			            if (semop(ctrCon->idSem, &unlockOperation, 1) < 0) {
			                fprintf(stderr, "No fue posible soltar el semaforo: %d %s\n",
			                errno, strerror(errno));
			                exit(1);
			            }
			    		/*** FIN SECCIÓN CRÍTICA***/
					}
						
				}

}

void* readErr(void *ptr){
	ctrlConcurrencia *ctrCon = (ctrlConcurrencia*) ptr;
	int rv=1;

	
	/* Semaforo 2, indice 1 */
	lockOperation = fnlockOperation(ctrCon->indiceSem);
	unlockOperation = fnunlockOperation(ctrCon->indiceSem);
	

	            while(rv>0){
	            	char line[MAXLINE];
					rv = read(*ctrCon->ty, line, MAXLINE);
					if(rv>0){
						/*** SECCIÓN CRÍTICA EN IMPRESIÓN (COUT/CERR)***/
			            if (semop(ctrCon->idSem, &lockOperation, 1) < 0) {
			                fprintf(stderr, "No fue posible senalar el semaforo: %d %s\n",
			                errno, strerror(errno));
			                exit(1);
			            }
			            write(2,line,rv);
						//Realizar operación en semáforo
			            if (semop(ctrCon->idSem, &unlockOperation, 1) < 0) {
			                fprintf(stderr, "No fue posible soltar el semaforo: %d %s\n",
			                errno, strerror(errno));
			                exit(1);
			            }
			    		/*** FIN SECCIÓN CRÍTICA***/
					}
						
				}

}



/*Devuelve parametros para Lock con indice de semaforo especifico */
sembuf fnlockOperation(int numeroSemaforo){
	struct sembuf lockOperation = {numeroSemaforo,-1,0};
	return lockOperation;
}

/*Devuelve parametros para unLock con indice de semaforo especifico */
sembuf fnunlockOperation(int numeroSemaforo){
	struct sembuf unlockOperation = {numeroSemaforo,1,0};
	return unlockOperation;
}


process getInfoProcess(string s){
	string tmp;
	size_t pos1,pos2;
	process p;
	pos1 = s.find(" ");
	s = s.substr(pos1+1); 				   //erase the first part 'ProcesoSui'
	pos1 = s.find(" ");
	p.id = s.substr(0,pos1); 			   //get the id
	pos1 = s.find("{");
	pos2 = s.find("::");
	p.path = s.substr(pos1+2,pos2-pos1-3); //get the path
	//Arreglamos si esa malo
	while(p.path[0]==' '){
		p.path.erase(p.path.begin());
	}
	while(p.path[p.path.size()-1]==' '){
		p.path.erase(p.path.end()-1);
	}
	tmp = s.substr(pos2+3);
	pos1 = tmp.find(" ");
	p.fileName = tmp.substr(0,pos1);	   //get the fileName
	tmp = tmp.substr(pos1+1);
	pos1 = tmp.find(" ");
	p.lives = atoi(tmp.substr(0,pos1).c_str());	//get the # lives
	return p;
}

/* Validacion de archivo de entrada */
bool checkValid(process p){
       if(p.id.empty())
               return false;
       else if(p.path.empty() || p.path.find("}")!=string::npos)
               return false;
       else if(p.fileName.empty())
               return false;
       else if(p.lives<0)
               return false;
       else
               return true;
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

void printInfoProcess(process p){	
	cout<<"Proceso id:"<<p.id<<"|"<<endl;
	cout<<"Proceso path:"<<p.path<<"|"<<endl;
	cout<<"Proceso fileName:"<<p.fileName<<"|"<<endl;
	cout<<"Proceso lives:"<<p.lives<<"|"<<endl;
	cout<<"\t-----------"<<endl;
}