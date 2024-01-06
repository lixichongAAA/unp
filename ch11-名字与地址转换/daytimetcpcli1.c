#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
    int sockfd, n;
    char recvline[MAXLINE + 1];
    char str[INET_ADDRSTRLEN];
    struct sockaddr_in servaddr;
    struct in_addr **pptr;
    struct in_addr *inetaddrp[2];
    struct in_addr inetaddr;
    struct hostent *hp;
    struct servent *sp;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: bin/daytimetcpcli1 <hostname> Mservice>");
        exit(1);
    }

    if ((hp = gethostbyname(argv[1])) == NULL)
    {
        if (inet_aton(argv[1], &inetaddr) == 0)
        {
            fprintf(stderr, "hostname error for %s: %s", argv[1], hstrerror(h_errno));
            exit(1);
        }
        else
        {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    }
    else
    {
        pptr = (struct in_addr **)hp->h_addr_list;
    }

    if ((sp = getservbyname(argv[2], "tcp")) == NULL)
    {
        fprintf(stderr, "getservbyname error for %s", argv[2]);
        exit(1);
    }

    for (; *pptr != NULL; ++pptr)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = sp->s_port;
        memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
        printf("trying %s\n",
               inet_ntop(hp->h_addrtype, &servaddr, str, sizeof(servaddr)));

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
            break;
        fprintf(stderr, "connect error");
        close(sockfd);
    }
    if (*pptr == NULL)
        fprintf(stderr, "unable to connect");

    while ((n = read(sockfd, recvline, MAXLINE)) > 0)
    {
        recvline[n] = 0;
        fputs(recvline, stdout);
    }

    return 0;
}