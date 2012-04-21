#include "libs.h"

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process;

//Functions
string intToStr(int i);
process getInfoProcess(string s);
void printInfoProcess(process p);
void* fConctrl(void *ptr);
void* read(void *ptr);

int main(){

	vector<process> processes; //Vector with all the information about the processes
	int nHilos = 0;           
    pthread_t *tablaDeHilos;  // Thread information

    //Read info from ArchCfg.txt
	string s;
	freopen("../Common/ArchCfg.txt", "r", stdin);
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

	/*
	procesoctrl --filepath=<path del ejecutable>
				--filename=<nombre del ejecutable> 
				--reencarnacion=<número de reencarnaciones>
				<número del proceso de control>
	*/
    //string parametros = p->id+" "+p->path+" "+p->fileName+" "+strToInt(p->lives)+" "+strToInt(p->control);

	int retVal;
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

	  execl("procesoctrl", "procesoctrl", p->id.c_str(),p->path.c_str(),p->fileName.c_str(),intToStr(p->lives).c_str(),intToStr(p->control).c_str(), (char *) 0);
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

        pthread_create(&readingIn, NULL, &read, (void *) &out[0]);
        pthread_join(readingIn,NULL);
        pthread_create(&readingErr, NULL, &read, (void *) &err[0]);
        pthread_join(readingErr,NULL);

	  wait(&retVal);
	  // Verifica si el hijo terminó bien
	  if (WIFEXITED(retVal)) {
	    //fprintf(stdout, "El proceso terminó bien: %d\n", 
	    //WEXITSTATUS(retVal));
	  }
	  else if (WIFSIGNALED(retVal)) { // Fue señalizado
	    //fprintf(stderr, "La señal capturada: %d\n",
	    //WTERMSIG(retVal));
	  }
	  else if (WIFSTOPPED(retVal)) {
	    //fprintf(stderr, "El proceso se encuentra parado: %d\n",
	    //WSTOPSIG(retVal));
	  }
	}

    //fCreateProcess(p);
}

 string intToStr(int i){
      string s;
      stringstream out;
      out << i;
      s = out.str();
      return s;
}

void* read(void *ptr){
	int *in = (int*) ptr;
	int rv=1;

	char line[MAXLINE];

	while(rv>0){
		rv = read(*in, line, MAXLINE);
		cout<<line;
	}
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