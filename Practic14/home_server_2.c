#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#define PORT 8800
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
    signal(SIGPIPE, SIG_IGN);
}

int main()
{
    int server, client;
    struct sockaddr_in local, remote;
    char str_buf[LEN_BUF];
    set_handler();

    if ((server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: start");
        exit(1);
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);

    
    if (bind(server, (struct sockaddr *)&local, sizeof(local)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    socklen_t len_remote;
    while (is_end != 1)
    {
        printf("SERVER: Start waiting on port %d.\n", PORT);
        len_remote = sizeof(remote);
        if ((client = accept(server, (struct sockaddr *)&remote, &len_remote)) == -1)
        {
            if (errno == EINTR) continue;
            perror("accept");
            exit(1);
        }

        printf("SERVER: Connection successful from %s:%d.\n", inet_ntoa(remote.sin_addr),
                                                            ntohs(remote.sin_port));

        while (is_end != 1)
        {
            ssize_t n = recv(client, str_buf, LEN_BUF - 1, 0);
            if (n < 0)
            {
                if (errno == EINTR) continue;
                perror("recv");
                break;
            } else if (n == 0)
            {
                printf("SERVER: Client closed connection.\n");
                break;
            }
            
            str_buf[n] = '\0';

            if (!is_end)
            {
                printf("SERVER: Accepted message: %s", str_buf);
                char *end_ptr;
                double x = strtod(str_buf, &end_ptr);
                if ((end_ptr != str_buf) && (errno != ERANGE))
                {
                    double y = x * x * x;
                    snprintf(str_buf, LEN_BUF, "%f", y);
                    
                    printf("SERVER: Sent cubed value.\n");
                } else
                {
                    if (errno == ERANGE) printf("SERVER: Accepted value is big or small!\n");
                    else printf("SERVER: Not find number.\n");
                    snprintf(str_buf, LEN_BUF, "No accessed value.\n");
                }
                
                if (send(client, str_buf, strlen(str_buf), 0) == -1)
                {
                    perror("send");
                    break;
                }
            }
        }
        close(client);
    }

    close(server);
    return 0;
}