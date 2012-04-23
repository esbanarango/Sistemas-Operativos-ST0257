#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int
main(int argc, char *argv[]) {

  int value1 = 10;
  int value2 = 20;
  DWORD pid;
  HANDLE hProcess;
  PCHAR pchar = NULL;

  pid = GetCurrentProcessId();

  srand(time(NULL));
  __try {
    switch (rand () % 5) {
    case 0:
      ExitProcess(0);
      break;
    case 1:
      value1 /= (value2 - value2);
      break;
    case 2:
      hProcess = OpenProcess(0, FALSE, pid);
      TerminateProcess(hProcess, 0);
      break;
    case 3:
      __asm sti;
      break;
    case 4:
      *pchar = '\0';
      break;
    }
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
    switch (GetExceptionCode()) {
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      ExitProcess(1);
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      ExitProcess(3);
      break;
    case EXCEPTION_ACCESS_VIOLATION:
      ExitProcess(4);
      break;
    default:
      ExitProcess(2);
      break;
    }
  }
  return 0;
}
