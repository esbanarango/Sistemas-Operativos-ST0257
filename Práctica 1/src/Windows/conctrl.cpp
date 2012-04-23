
#include "libs.h"

#define BUFFSIZE 1024

/* Process struct  */
typedef struct str_process{
  string id,path,fileName;
  int lives,control;
}process, *pProcess;

/*  Funciones */
process getInfoProcess(string s);
bool checkValid(process p);
string IntToStr(int i);
DWORD WINAPI fConctrl(LPVOID lpParam);
DWORD WINAPI read(LPVOID lpParam);
void printInfoProcess(process p);

int main(){

    DWORD dwResultado;
    int i;
    DWORD *tablaHilos;
	vector<process> processes; //Vector con toda la informacion del proceso
	int nHilos = 0; 
    pProcess pDataArray[nHilos];
         

    //Read info from ArchCfg.txt
	string s;
	freopen("../Common/ArchCfgw.txt", "r", stdin);
    while (getline(cin,s)){
  		process p = getInfoProcess(s);
  		if (!checkValid(p)){
  			printf("Por favor revise el archivo 'ArchCfg.txt', este esta mal formado.\n");
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

     tablaHilos = (LPDWORD) malloc(sizeof(LPDWORD) * nHilos);
	           
    
    for (i = 0; i < nHilos; i++) {
         pDataArray[i] = (pProcess) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(process));
           
      if( pDataArray[i] == NULL )
        {
           printf("pDataArray NULL");
            ExitProcess(2);
        } 
                         
        processes[i].control = i+1;         
        process p = processes[i];
        pDataArray[i] = &p;

         HANDLE hThread;
         hThread= CreateThread(NULL,
                     0,
                     fConctrl,
                     pDataArray[i],
                     0,
                     (tablaHilos + i));
                     
                     
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &dwResultado);
    }

          
    return dwResultado;

    }

/* Funcion de Hilo  */
DWORD WINAPI fConctrl( LPVOID lpParam ) {
      
      /* Handlers */
    HANDLE g_hChildStd_OUT_Rd;
    HANDLE g_hChildStd_OUT_Wr;
    HANDLE g_hChildStd_IN_Rd;
    HANDLE g_hChildStd_IN_Wr;
    HANDLE g_hChildStd_ERR_Rd;
    HANDLE g_hChildStd_ERR_Wr;
    HANDLE hStdOutput;

    DWORD *tablaHilosLectura;
    tablaHilosLectura = (LPDWORD) malloc(sizeof(LPDWORD) * 2);

    DWORD dwResultado = 0;
    DWORD dwExitCode;

   /* Tomar el parametro y castearlo a estructura Process */
    process *p;
    p = (process *)lpParam;
    /* Generar el string con los parametros */
    string executable = " "+p->id+" "+ p->path+" "+ p->fileName +" "+IntToStr(p->lives)+" "+IntToStr(p->control);
    
   
      STARTUPINFO startupInfo;
      PROCESS_INFORMATION piProcInfo;
      HANDLE hReadPipe, hWritePipe, hErrorWrite;
      SECURITY_ATTRIBUTES saAttr;

      saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
      saAttr.bInheritHandle = TRUE;
      saAttr.lpSecurityDescriptor = NULL;
      
      
       /* Crear pipe para la salida estandar */
        if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) {
          fprintf(stderr, "Error creando std_out pipe");
          }
    
       /* Crear pipe para la entrada estandar */
        if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
           fprintf(stderr, "Error creando std_in pipe");
           }
         
       /* Crear pipe para el error estandar */
        if ( ! CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &saAttr, 0) ){
           fprintf(stderr, "Error creando std_err pipe");
           }
     
      /* Inicializando las estructuras de la informacion de los procesos */
      ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    
      /* Inicializando la informacion que cada proceso necesita */
      ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
      startupInfo.cb = sizeof(STARTUPINFO);
      startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
      startupInfo.hStdOutput = g_hChildStd_OUT_Wr;
      startupInfo.hStdError = g_hChildStd_ERR_Wr;
      startupInfo.dwFlags |= STARTF_USESTDHANDLES;
      
      /* Crear proceso control */
      if (CreateProcess("procesoctrl.exe", const_cast<LPTSTR>(executable.c_str()),
       NULL, NULL,TRUE, 0, NULL, NULL, &startupInfo, &piProcInfo)) { 
                  WaitForSingleObject(piProcInfo.hProcess, 0);
                  dwExitCode = GetExitCodeProcess(piProcInfo.hProcess, &dwExitCode);
      }
      else {
           printf( "CreateProcess fallo (%d).\n", GetLastError() );
            dwExitCode = GetLastError();
      }

        /* Release handles */
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread); 
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_ERR_Wr);
    
        HANDLE hThreadReadOut;
        HANDLE hThreadReadErr;
        
        /* Hilo para la salida estandar */
        hThreadReadOut= CreateThread(NULL,
                     0,
                     read,
                     (LPVOID) &(g_hChildStd_OUT_Rd),
                     0,
                     NULL);
                    
                     
        WaitForSingleObject(hThreadReadOut, INFINITE);
    
        /* Hilo para el error estandar */
        hThreadReadErr= CreateThread(NULL,
                     0,
                     read,
                     (LPVOID) &(g_hChildStd_ERR_Rd),
                     0,
                     NULL);
                     
        WaitForSingleObject(hThreadReadErr, INFINITE);

        
    return dwExitCode;
      
      
}

/* Funcion de lectura de buffer en hilos */
DWORD WINAPI read(LPVOID lpParam){
      
      /* Handler parametro de lectura */
        HANDLE *read= (HANDLE*) lpParam;
        HANDLE hStdOutput;      
       	DWORD dwRead, dwWritten, dwExitCode; 
        CHAR chBuf[BUFFSIZE]; 
        hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        if( hStdOutput == INVALID_HANDLE_VALUE ){
            return 1;
        }
            
        while (ReadFile(*read,            // hFile
                       (PVOID) chBuf,      // pvBuffer
            		   (DWORD) BUFFSIZE,   // nNumBytesToRead
                               &dwRead,    // pdwNumBytes
                               NULL        // pOverlapped
                    )) {
                    if (dwRead == 0) {
                      break;
                    }
                 
                    if (!WriteFile(hStdOutput,       // hFile
                		    (PVOID) chBuf,           // pvBuffer
                                    dwRead,          // pdwNumBytes
                                    &dwWritten,      // pdwNumBytes
                                    NULL)) {
                      printf("ProcesoCtrl - Error escribiendo: %ld\r\n", GetLastError());
                      ExitProcess((DWORD) 1);
                    }    
          }
}


/* Obtener la informacion del proceso */
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


/* Transforma un entero a un string */
string IntToStr(int i){
       string s;
       stringstream out;
       out << i;
       s = out.str();
       return s;
}


void printInfoProcess(process p){	
	cout<<"Proceso id:"<<p.id<<"|"<<endl;
	cout<<"Proceso path:"<<p.path<<"|"<<endl;
	cout<<"Proceso fileName:"<<p.fileName<<"|"<<endl;
	cout<<"Proceso lives:"<<p.lives<<"|"<<endl;
	cout<<"\t-----------"<<endl;
}

