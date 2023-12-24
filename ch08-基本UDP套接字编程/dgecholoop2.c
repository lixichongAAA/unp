#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define SERV_PORT 9877
#define MAX_LINE 4096

static void recvfrom_int(int);
static int count;

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clntlen);

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr, clntaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    dg_echo(sockfd, (struct sockaddr *)&clntaddr, sizeof(clntaddr));

    return 0;
}

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clntlen)
{
    int n;
    socklen_t len;
    char msg[MAX_LINE];

    signal(SIGINT, recvfrom_int);

    n = 220 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, clntlen);

    for (;;)
    {
        len = clntlen;
        recvfrom(sockfd, msg, MAX_LINE, 0, pcliaddr, &len);
        count++;
    }
}

static void recvfrom_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}