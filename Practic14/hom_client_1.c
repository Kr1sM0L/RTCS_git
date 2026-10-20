#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>


#define SOCK_PATH "socket_unix"
#define LEN_BUF 256


static volatile sig_atomic_t is_end = 0;


void int_handler(int signo)
{
    (void)signo;
    fprintf(stdout, "\nSignal %d accepted. End flag in up state.\n", signo);
    is_end = 1;
}

void set_handler()
{
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

int main()
{
    set_handler();
    int s, end_id;
    struct sockaddr_un remote;
    char rec_str[LEN_BUF];
    char send_str[LEN_BUF];

    printf("CLIENT: Start working.\n");
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    printf("CLIENT: Trying connect.\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1)
    {
        perror("connect");
        exit(1);
    }
    printf("CLIENT: Connected.\n");

    while (printf("> "), fgets(send_str, LEN_BUF, stdin) != NULL)
    {
        if (send(s, send_str, strlen(send_str), 0) == -1)
        {
            perror("send");
            break;
        }

        if (is_end) break;
        if ((end_id = recv(s, rec_str, LEN_BUF, 0)) > 0)
        {
            rec_str[end_id - 1] = '\0';
            printf("CLIENT: Received string: |%s|\n", rec_str);
        } else
        {
            if (end_id < 0) perror("recv");
            else printf("CLIENT: Server closed connection.\n");
            break;
        }

        if (is_end) break;
    }
    printf("CLIENT: Stop working.\n");

    close(s);

    return 0;
}