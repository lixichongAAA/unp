#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SERV_PORT 9877
#define MAX_LINE 4096

void dg_echo(int sockfd, struct sockaddr* pcliaddr, socklen_t clntlen);

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_in servaddr, clntaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    dg_echo(sockfd, (struct sockaddr*)&clntaddr, sizeof(clntaddr));

    return 0;
}

void dg_echo(int sockfd, struct sockaddr* pcliaddr, socklen_t clntlen)
{
    int n;
    socklen_t len;
    char mesg[MAX_LINE];

    for(;;)
    {
        len = clntlen;
        n = recvfrom(sockfd, mesg, MAX_LINE, 0, pcliaddr, &len);
        sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
}