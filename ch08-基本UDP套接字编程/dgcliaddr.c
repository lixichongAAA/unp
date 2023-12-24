#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
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
    socklen_t len;
    struct sockaddr* preply_addr;
    char str[INET_ADDRSTRLEN];

    preply_addr = malloc(servlen);
    while(fgets(sendline, MAX_LINE, fp) != NULL)
    {
        sendto(sockfd, sendline, strlen(sendline), 0, servaddr, servlen);

        len = servlen;
        n = recvfrom(sockfd, recvline, MAX_LINE, 0, preply_addr, &len);
        if(len != servlen || memcmp(servaddr, preply_addr, len) != 0)
        {
            struct sockaddr_in* sin = (struct sockaddr_in*)preply_addr;

            printf("reply from %s (ignored)\n", inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
            continue;
        }
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}