#include "libs.h"

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process;

//Functions
string coloriar(int color, string texto);
process getInfoProcess(string s);
void printInfoProcess(process p);
bool checkValid(process p);
void* fConctrl(void *ptr);
string intToStr(int i);
void* read(void *ptr);


int main(){

	vector<process> processes; //Vector with all the information about the processes
	int nHilos = 0;           
    pthread_t *tablaDeHilos;  // Thread information

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
    string id = "--id="+p->id;
    string filepath="--filepath="+p->path;
    string filename="--filename="+p->fileName;
    string reencarnacion="--reencarnacion="+intToStr(p->lives);
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

	  execl("procesoctrl", "procesoctrl", id.c_str(),filepath.c_str(),filename.c_str(),reencarnacion.c_str(),contrlNum.c_str(), (char *) 0);
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
    }
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
		if(rv>0)
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