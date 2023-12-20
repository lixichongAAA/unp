#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>

int main(int argc, char* argv[])
{
    char ip[] = "134.234.123.111";
    u_int32_t result;
    inet_pton(AF_INET, ip, &result);
    printf("result: %d\n", result);

    inet_ntop(AF_INET, &result, ip, sizeof(ip));
    printf("dotted decimal: %s\n", ip);
    
    return 0;
}