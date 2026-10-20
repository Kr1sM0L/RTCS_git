#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


#define PERMS 0644
#define LEN 256
#define FILE_DEST "mes_query"


volatile sig_atomic_t is_end = 0;


struct msgbuf {
    long mtype;
    pid_t pid;
    time_t timestamp;
    char mtext[256];
};


void int_handler() {
    printf("MSGQ receive: stopping program.\n");
    is_end = 1;
}

int main() {
    int msgqid;
    key_t key;
    struct msgbuf buf;

    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    FILE *f = fopen(FILE_DEST, "a");
    if (f == NULL) {
        perror("fopen");
        return -1;
    }
    fclose(f);

    if ((key = ftok(FILE_DEST, 'A')) == -1) {
        perror("ftok");
        return -1;
    }

    if ((msgqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
        perror("msgqid");
        return -1;
    }

    printf("MSGQ receive: Start working.\n");

    while (is_end != 1) {
        if (msgrcv(msgqid, &buf, sizeof(buf.mtext), 0, 0) == -1) {
            perror("msgrcv");
            return -1;
        }
        printf("MSGQ receive: %5ld priority | %4ld bytes | %10ld time | %5d pid.\nText: %s\n", 
                buf.mtype, sizeof(buf.mtext), buf.timestamp, buf.pid, buf.mtext);   
    }
    
    printf("MSGQ send: done.\n");
    system("rm msgq.txt");
    return 0;
}