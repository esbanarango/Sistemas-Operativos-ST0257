#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <fstream>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
#include <vector>
#include <cmath>
#include <queue>
#include <deque>
#include <stack>
#include <list>
#include <map>
#include <set>
using namespace std;
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define MAXLINE 1024

int main(int argc, char *argv[])
{
    int retVal;
    int fd1[2];
    int fd2[2];
    pid_t pid;
    char line[MAXLINE];

    pipe(fd1);
    pipe(fd2);

    if ( (pid = fork()) < 0 )
    {
        cerr << "FORK ERROR" << endl;
        return -3;
    }
    else  if (pid == 0)     // CHILD PROCESS
    {
        close(fd1[1]);
        close(fd2[0]);

        dup2(fd1[0], STDIN_FILENO);
        close(fd1[0]);

        dup2(fd2[1], STDOUT_FILENO);
        close(fd2[1]);

        if ( execl("pSuicida", "pSuicida", (char *)0) < 0 )
        {
            cerr << "system error" << endl;
            return -4;
        }

        return 0;
    }
    else        // PARENT PROCESS
    {
        int rv;
        close(fd1[0]);
        close(fd2[1]);

        if ( (rv = read(fd2[0], line, MAXLINE)) < 0 )
        {
            cerr << "READ ERROR FROM PIPE" << endl;
        }
        else if (rv == 0)
        {
            cerr << "Child Closed Pipe" << endl;
            return 0;
        }

        cout<<"Lei: "<< rv<<" lines"<<endl;
        cout << "OUTPUT of PROGRAM B is: \n" << line;


        int causa;
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
        cout<<"oe "<<causa;
    }
    return 0;
}