// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <signal.h>


// #define MAX_CHLD_PROC_NUM 10


// void child_exit_handler (int sig) {
//     pid_t ret;
//     int status = 0;
//     while((ret = waitpid( -1 , &status, 0 )) > 0 )
//     {
//         if (WIFEXITED(status)) { 
//             printf ( "Дочерний процесс %d завершился с кодом выхода %d\n" , ret, WEXITSTATUS(status));
//         } else if (WIFSIGNALED(status)){
//             printf ( "Дочерний процесс %d завершен сигналом %d\n" , ret,  WTERMSIG(status));
//         } else if (WIFSTOPPED(status)){ 
//             printf ( "Дочерний процесс %d остановлен сигналом %d\n" , ret, WSTOPSIG(status));
//         } else { 
//             printf ( "Дочерний процесс %d завершился с неизвестным кодом выхода\n" , ret);
//         }
//     }
// }

// int  main ( int argc, char ** argv){ 
//     pid_t pid[MAX_CHLD_PROC_NUM]; 
//     signal(SIGCHLD, child_exit_handler);

//     for (int i = 0; i < MAX_CHLD_PROC_NUM; i++){
//         if ((pid[i] = fork()) == 0 ){
//             char *child_argv[] = {"./child", NULL};
//             char *child_envp[] = {NULL};
//             execve("/media/daniil/Apacer/MIREA/VI_semester/RTCS/RTCS_git/Lab8/child", child_argv, child_envp);
//         }
//     } 
//     while ( 1 ) {
//         printf ( "Родительский процесс запущен\n" );
//         sleep( 2 );
//     }
//     exit(0);
// }
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_CHLD_PROC_NUM 10


int  main ( int argc, char ** argv){ 
    int status = 0;
    pid_t ret;
    pid_t pid[MAX_CHLD_PROC_NUM];

    for ( int i = 0 ; i < MAX_CHLD_PROC_NUM; i++) {
        if ((pid[i] = fork()) == 0)
        { 
            char *child_argv[] = {"./child", NULL};
            char *child_envp[] = {NULL};
            execve("/media/daniil/Apacer/MIREA/VI_semester/RTCS/RTCS_git/Lab8/child", child_argv, child_envp);
        } 
    }

    while ((ret = waitpid( -1 , &status, 0 )) > 0 ){ 
        if (WIFEXITED(status)) {
            printf( "дочерний процесс %d завершается с кодом выхода %d\n" , ret, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf ( "дочерний процесс %d завершен сигналом %d\n" , ret, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf ( "Дочерний процесс %d остановлен сигналом %d\n" , ret, WSTOPSIG(status));
        } else{
            printf ( "Дочерний процесс %d завершился с неизвестным исходным кодом\n" , ret); 
        }
    }
    printf("завершение родительского процесса\n");
    exit(0); 
}