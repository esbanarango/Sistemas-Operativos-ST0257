#include "libs.h"

#define BUFFSIZE 1024

/* Process structure */
typedef struct str_process{
  string id,path,fileName;
  int lifes,control;
}process, *pProcess;

/* Funciones */
void fCreateProcess(process *p);
string IntToStr(int i);
DWORD WINAPI read(LPVOID lpParam);


int main(int argc, char* argv[]){

    /* Obtener los argumentos */
    process p;
    p.id=argv[1];
    p.path=argv[2];
    p.fileName=argv[3];
    string lifes = argv[4];
    p.lifes=atoi(lifes.c_str());
    string control = argv[5];
    p.control=atoi(control.c_str());
    fCreateProcess(&p);

return 0;

}

void fCreateProcess(process *p){
     do{         
       	              
    /* Handlers */
    HANDLE g_hChildStd_IN_Rd;
    HANDLE g_hChildStd_IN_Wr;
    HANDLE g_hChildStd_OUT_Rd;
    HANDLE g_hChildStd_OUT_Wr;
    HANDLE g_hChildStd_ERR_Rd;
    HANDLE g_hChildStd_ERR_Wr;

    DWORD dwExitCode;
    DWORD dwResultado;

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION piProcInfo;
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
     
      /* Inicializando las estructuras de la información de los procesos */
      ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    
      /* Inicializando la información que cada proceso necesita */
      ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
      startupInfo.cb = sizeof(STARTUPINFO);
      startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
      startupInfo.hStdOutput = g_hChildStd_OUT_Wr;
      startupInfo.hStdError = g_hChildStd_ERR_Wr;
      startupInfo.dwFlags |= STARTF_USESTDHANDLES;
      
         /*Crear Proceso Suicida */
          if (CreateProcess(NULL, const_cast<LPTSTR>(p->path.c_str()), NULL, NULL,
               TRUE, 0, NULL, NULL, &startupInfo, &piProcInfo)) { 
                     WaitForSingleObject(piProcInfo.hProcess, INFINITE);
                     GetExitCodeProcess(piProcInfo.hProcess, &dwExitCode);
                 
          }
          else {
               printf( "CreateProcess fallo (%d).\n", GetLastError() );
               dwExitCode = GetLastError();
          }
          
        /* Soltar Handlers */
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
	p->lifes--;	
	
	cout<<"Proceso suicida "<<p->id<<" termino por causa  "<<dwExitCode<<" -- Proceso Control "<<p->control<<", vidas restantes: "<<((p->lifes<0)?"Infinitas":IntToStr(p->lifes))<<endl;
	fflush(stdout);
	}while(p->lifes != 0);

}

/* Función que convierte un entero a string */
string IntToStr(int i){
       string s;
       stringstream out;
       out << i;
       s = out.str();
       return s;
}

/* Lectura de buffers */
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

