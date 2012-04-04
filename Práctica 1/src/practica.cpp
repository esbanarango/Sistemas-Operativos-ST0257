#include "libs.h"

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process;

//Functions
process getInfoProcess(string s);
void printInfoProcess(process p);
void* fConctrl(void *ptr);
void fCreateProcess(process *p);

int main(){

	vector<process> processes; //Vector with all the information about the processes
	int nHilos = 0;           
    pthread_t *tablaDeHilos;  // Información de los hilos

    //Read info from ArchCfg.txt
	string s;
	freopen("ArchCfg.txt", "r", stdin);
  	while (getline(cin,s)){
  		processes.push_back(getInfoProcess(s));
  		nHilos++;
	}

	tablaDeHilos = (pthread_t *) malloc(sizeof(pthread_t) * nHilos);
	cout<<"Total hilos:"<<nHilos<<endl;
	//Execute each process
	for (int i = 0; i < nHilos; i++){
		processes[i].control = i+1;
		pthread_create((tablaDeHilos + i), NULL, &fConctrl, (void *) &processes[i]);
	}

	for (int i = 0; i < nHilos; i++) {
	    pthread_join(*(tablaDeHilos +i), NULL);
	}
    	
	//naive way
	/*for (int i = 0; i < processes.size(); ++i)
	{
		int total = processes[i].lives;
		fCreateProcess(&processes[i]);
	}*/

	return 0;
}



/******** HELPERS FUNCTIONS ********/

void* fConctrl(void *ptr){
	process *p;            
    p = (process *) ptr;  /* type cast to a pointer to process */
    fCreateProcess(p);
}

void fCreateProcess(process *p){

	int retVal;
	pid_t pid;
	int causa;
	pid = fork();

	if (pid == (pid_t)(-1)) {
		fprintf(stderr, "%s, Fallo al hacer el fork\n",
		    strerror(errno));
		exit(13);
	}
	else if (pid == 0) {
		close(1);
		execl(p->path.c_str(), p->fileName.c_str(), NULL, (char *) 0);
		// No se debe ejecutar este código
		fprintf(stderr, "No pudo ejecutar %s %s\n",p->path.c_str(),strerror(errno));
	}
	else {
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
	}
	p->lives--;	
	// Messages
	cout<<"Proceso suicida "<<p->id<<" termino por causa "<<causa<<" -- Proceso Control "<<p->control<<", vidas restantes: "<<p->lives<<endl;
	if(p->lives != 0)
		fCreateProcess(p);
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
	tmp = s.substr(pos2+3);
	pos1 = tmp.find(" ");
	p.fileName = tmp.substr(0,pos1);	   //get the fileName
	tmp = tmp.substr(pos1+1);
	pos1 = tmp.find(" ");
	p.lives = atoi(tmp.substr(0,pos1).c_str());	//get the # lives
	return p;
}

/*printf("Proceso %d:\n",i+1);
  		printInfoProcess(processes[i]);
		i++;*/
void printInfoProcess(process p){	
	cout<<"Proceso id:"<<p.id<<endl;
	cout<<"Proceso path:"<<p.path<<endl;
	cout<<"Proceso fileName:"<<p.fileName<<endl;
	cout<<"Proceso lives:"<<p.lives<<endl;
	cout<<"\t-----------"<<endl;
}