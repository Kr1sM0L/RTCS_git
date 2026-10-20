#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int  main ( int argc, char ** argv)
{ 
    while (1)
    { 
        printf ("дочерний процесс: PID = %d sleep.\n", getpid());
        sleep( 10 );
        printf ("дочерний процесс: PID = %d wakeup.\n", getpid());
    }
    printf ("дочерний процесс: PID = %d завершен.\n", getpid());
    exit (0);
}