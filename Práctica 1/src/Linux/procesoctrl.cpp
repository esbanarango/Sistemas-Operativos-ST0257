#include "libs.h"
#include <stdlib.h>

//Process struct it has all the information about a process
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process;

process getParamsToProcess(int argc, char *argv[]);
string coloriar(int color, string texto);
void fCreateProcess(process *p);
void* readOut(void *ptr);
void* readErr(void *ptr);
string intToStr(int i);



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
         }
         cout<<"Proceso suicida "<<coloriar(VERDE,p->id)<<" termino por causa ";
    	 cout<<coloriar(ROJO,intToStr(causa))<<" -- Proceso Control "<<coloriar(AMARILLO,intToStr(p->control))<<", vidas restantes: "<<((--p->lives<0)?"infinitas":intToStr(p->lives))<<endl; 
    }while(p->lives != 0);
     /*if(p->lives<0)
            cout<<"Vidas vividas: "<<p->lives*-1;
     cout<<endl;
    if(p->lives != 0)
		fCreateProcess(p);*/

}

void* readErr(void *ptr){
    int *in = (int*) ptr;
    int rv=1;

    char line[MAXLINE];

    while(rv>0){
        rv = read(*in, line, MAXLINE);
        if(rv>0)
            write(2,line,rv);
    }
}

void* readOut(void *ptr){
    int *in = (int*) ptr;
    int rv=1;

    char line[MAXLINE];

    while(rv>0){
        rv = read(*in, line, MAXLINE);
        if(rv>0)
            write(1,line,rv);
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