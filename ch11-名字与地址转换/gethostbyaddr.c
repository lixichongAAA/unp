#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
    struct in6_addr ipv6_address;
    const char *ipv6_str = "fe80::d3ce:c6d0:19da:88f5"; // 示例 IPv6 地址
    inet_pton(AF_INET6, ipv6_str, &ipv6_address);

    struct hostent *host_info;

    // 获取主机信息
    host_info = gethostbyaddr(&ipv6_address, sizeof(ipv6_address), AF_INET6);

    if (host_info == NULL)
    {
        hstrerror(h_errno);
        return 1;
    }

    // 打印主机名和对应的 IPv6 地址
    printf("Host: %s, IP: %s\n", host_info->h_name, ipv6_str);

    return 0;
}
