#include "libs.h"
#include <stdlib.h>



//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process;

void fCreateProcess(process *p);
string intToStr(int i);
string coloriar(int color, string texto);

int main(int argc, char *argv[]){

	//Make a new process
	process p;
	p.id = argv[1]; 			   
	p.path = argv[2]; 
	p.fileName = argv[3]; 	   
	p.lives = atoi(argv[4]);	
	p.control = atoi(argv[5]);	

	fCreateProcess(&p);

	return 0;

}


void fCreateProcess(process *p){


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
        close(in[0]);
        close(out[1]);
        close(err[1]);
        /*
        char line[MAXLINE];
        
        if ( (rv = read(out[0], line, MAXLINE)) < 0 )
        {
            cerr << "READ ERROR FROM PIPE" << endl;
        }


        cout<<"Lei: "<< rv<<" lines"<<endl;
        cout << "Para mi papa: "<< p->id <<" is: \n" << line;
        */
        
        //fprintf(stderr, "Probando el error");

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
     cout<<"Proceso suicida "<<coloriar(VERDE,p->id)<<" termino por causa ";
	 cout<<coloriar(ROJO,intToStr(causa))<<" -- Proceso Control "<<coloriar(AMARILLO,intToStr(p->control))<<", vidas restantes: "<<--p->lives<<endl; 
    if(p->lives != 0)
		fCreateProcess(p);

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