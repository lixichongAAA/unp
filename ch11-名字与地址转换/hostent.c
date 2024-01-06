#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    char *ptr, **pptr;
    char str[INET_ADDRSTRLEN];
    struct hostent *hptr;

    while (--argc > 0)
    {
        ptr = *++argv;
        if ((hptr = gethostbyname(ptr)) == NULL)
        {
            printf("gethostbyname error for host: %s: %s", ptr, hstrerror(h_errno));
            continue;
        }

        printf("Official name: %s\n", hptr->h_name);
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        {
            printf("\taddress alias: %s\n", *pptr);
        }

        switch (hptr->h_addrtype)
        {
        case AF_INET:
            pptr = hptr->h_addr_list;
            for (; *pptr != NULL; pptr++)
                printf("\tIPv4 address: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            break;
        default:
            fputs("unknown address type", stderr);
            break;
        }
    }

    return 0;
}