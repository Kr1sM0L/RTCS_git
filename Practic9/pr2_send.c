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
    printf("MSGQ send: stopping program.\n");
    is_end = 1;
}

int main() {
    struct msgbuf buf;
    int msqid;
    key_t key;
    char inp_val[LEN+2];

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

    if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
        perror("msgget");
        return -1;
    }

    printf("MSGQ send: Start program.\nEnter Ctrl+C to stop program.\n");
    printf("Send message as: [priority] [text]\nExample: > 5 Hello\n");
    
    buf.pid = getpid();
    
    while (is_end != 1) {
        int msg_prior;
        char msg_buf[LEN];
        printf("> ");

        if (fgets(inp_val, sizeof(inp_val), stdin) == NULL) {
            continue;
        }
        
        if (sscanf(inp_val, "%d %[^\n]", &msg_prior, msg_buf) < 2) {
            printf("Type message as: [priority] [text]\n");
            continue;
        }
        
        if (msg_buf[LEN] == '\n') {
            msg_buf[LEN] = '\0';
        }

        buf.mtype = msg_prior;
        buf.timestamp = time(NULL);
        strncpy(buf.mtext, msg_buf, sizeof(msg_buf) - 1);

        if (msgsnd(msqid, &buf, LEN, 0) == -1) {
            perror("msgsnd");
            return -1;
        } else {
            printf("MSGQ send: %5ld priority | %4ld bytes | %10ld time | %5d pid.\n", 
                buf.mtype, sizeof(buf.mtext), buf.timestamp, buf.pid);
        }
    }

    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl: rmid");
        return -1;
    }

    printf("MSGQ send: done.\n");
    return 0;
}