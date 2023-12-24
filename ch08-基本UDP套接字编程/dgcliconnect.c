#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LINE 4096
#define SERV_PORT 9877

void dg_cli(FILE* fp, int sockfd, const struct sockaddr* servaddr, socklen_t servaddrlen);

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 2)
    {
        printf("Usage: %s <IPaddress>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    dg_cli(stdin, sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    return 0;   
}

void dg_cli(FILE* fp, int sockfd, const struct sockaddr* servaddr, socklen_t servlen)
{
    int n;
    char sendline[MAX_LINE], recvline[MAX_LINE + 1];

    connect(sockfd, (struct sockaddr*)servaddr, servlen);
    while(fgets(sendline, MAX_LINE, fp) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));

        n = read(sockfd, recvline, MAX_LINE);
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}