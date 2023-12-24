#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/select.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define SERV_PORT 9877
#define MAX_LINE 4096
#define LISTENQ 1024

#define	max(a,b)	((a) > (b) ? (a) : (b))

void str_echo(int sockfd);
void sig_chld(int signo);

int main(int argc, char *argv[])
{
    int listenfd, connfd, udpfd, nready, maxfdp1;
    char mesg[MAX_LINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    signal(SIGCHLD, sig_chld);

    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;
    for (;;)
    {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        if ((nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                exit(1);
        }

        if (FD_ISSET(listenfd, &rset))
        {
            len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);

            if ((childpid = fork()) == 0)
            {
                close(listenfd);
                str_echo(connfd);
                exit(0);
            }
            close(connfd);
        }

        if (FD_ISSET(udpfd, &rset))
        {
            len = sizeof(cliaddr);
            n = recvfrom(udpfd, mesg, MAX_LINE, 0, (struct sockaddr*)&cliaddr, &len);
            sendto(udpfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len);
        }
    }

    return 0;
}

void str_echo(int sockfd)
{
    ssize_t n;
    char buf[MAX_LINE];

again:
    while ((n = read(sockfd, buf, MAX_LINE)) > 0)
        write(sockfd, buf, n);

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        exit(1);
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}