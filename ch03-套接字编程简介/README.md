# ch03 套接字编程简介

## 3.2 套接字地址结构

大多数套接字函数都需要一个指向套接字地址结构的指针作为参数。每个协议族都定义它自己的套接字地址结构。这些结构的名字均以 `sockaddr_` 开头，并以对应每个协议族的唯一后缀结尾。

### 3.2.1 IPv4套接字地址结构

它以 `sockaddr_in` 命名，定义在 `<netinet/in.h>` 头文件中。下面给出了它的POSIX定义。

```c
struct sockaddr_in
{
    in_addr_t s_addr; // 32-bit IPv4 address
                      // network byte ordered
}

struct sockaddr_in
{
    uint8_t sin_len;        // length of structure(16)
    sa_family_t sin_family; // AF_INET
    in_port_t sin_port;     // 16-bit TCP or UDP port number
                            // betwork byte ordered
    struct in_add sin_addr; // 32-bit IPv4 address
                            // network byte ordered
    char sin_zero[8];       // unused
}
```

- 长度字段 `sin_len` 是为了增加对OSI协议的支持而随 4.3BSD-Reno 添加的。不是所有厂家都支持该字段，而且POSIX规范也不要求有这个成员。对于该长度字段，我们无需设置和检查它，除非涉及路由套接字（见第18章）。它是由来自不同协议族的套接字地址结构的例程（例如路由表处理代码）在内核中使用的。

- POSIX规范只需要这个结构中的3个字段：`sin_family`、`sin_addr` 和 `sin_port`。对于符合POSIX的实现来说，定义额外的结构字段是可以接受的，这对于网际套接字地址结构来说也是正常的。几乎所有实现都增加了 `sin_zero` 字段，所有现有的套接字地址结构的大小都至少是16字节。

- `in_addr_t` 数据类型必须是一个至少32位的无符号整型数，`in_port_t` 必须是一个至少16位的无符号整数类型，而 `sa_family_t` 可以是任何无符号整数类型。在支持长度字段的实现中，`sa_familt_t` 通常是一个8位的无符号整数，而在不支持长度字段的实现中，它则是一个16位的无符号整数。

- 对于数据类型 `u_char`、`u_short`、`u_int` 和 `u_long` 它们都是无符号的。POSIX规范定义这些类型时特地标记它们已过时，仅是为了向后兼容才提供的。

- **IPv4 地址和TCP或UDP端口号在套接字地址结构中总是以网络字节序来存储**，要牢记。

- 32位IPv4地址存在两种不同的访问方法。举例来说，如果 *serv* 定义为某个网际套接字地址结构，那么 *serv.sin_addr* 将按 `in_addr` 结构引用其中的32位IPv4地址，而 *serv.sin_addr.s_addr* 将按 `in_addr_t` (通常是一个无符号的32位整数)引用同一个IPv4地址。因此，我们必须正确地使用IPv4地址，尤其是在将它作为函数的参数时，因为编译器对传递结构和传递整数的处理是完全不同的。

- `sin_zero` 字段未曾使用，不过在填写这种套接字地址结构时，我们总是把该字段置为0。按照惯例，我们总是在填写前将整个结构置为0，而不是单单把该字段置为0。见 [TCP/IP网络编程的笔记](https://github.com/lixichongAAA/TCP-IP-Network-Programming/blob/master/ch03-%E5%9C%B0%E5%9D%80%E6%97%8F%E4%B8%8E%E6%95%B0%E6%8D%AE%E5%BA%8F%E5%88%97/README.md#4-%E6%88%90%E5%91%98-sin_zero)。

### 3.2.2 通用套接字地址结构

当作为一个参数传递进任何套接字函数时，套接字地址结构总是以引用形式（也就是以指向该结构的指针）来传递。然而以这样的指针作为参数之一的任何套接字函数必须处理来自支持的任何协议族的套接字地址结构。  
在如何声明所传递的指针类型上存在一个问题。有了ANSI C后很简单：`void* ` 是通用的指针类型。声明成该类型就完事儿了。然而套接字函数是在ANSI C之前定义，所以当时咋整的呢，在1982年采取的办法是在 `<sys/socket.h>` 头文件中定义一个通用的套接字地址结构，如下所示。

```c
struct sockaddr
{
    uint8_t sa_len;
    sa_family_t sa_family; // address family: AF_xxx value
    char sa_data[14];      // protocol-specific address
}
```

于是套接字函数被定义为以指向某个通用套接字地址结构的一个指针作为其参数之一。这就要求对于这些函数的调用都必须要将指向特定于协议的套接字地址结构的指针进行强制类型转换，变成指向某个通用套接字地址结构的指针。例如：

```c
struct sockaddr_in serv;

// 填充 serv 结构

bind(sockfd, (struct sockaddr*)&serv, sizeof(serv));
```

### 3.2.3 IPv6套接字地址结构

IPv6套接字地址结构在 `<netinet/in.h>` 头文件中定义，如下所示。

```c
struct in6_addr
{
    uint8_t s6_addr[16]; // 128-bit IPv6 address
                         // network byte ordered
}
#define SIN6_LEN // required for compile-time tests

struct sockaddr_in6
{
    uint8_t sin6_len;          // length of this struct(28)
    sa_family_t sin6_family;   // AF_INET6
    in_port_t sin6_port;       // transport layer port
                               // network byte ordered
    uint32_t sin6_flowinfo;    // flow information, undefined
    struct in6_addr sin6_addr; // IPv6 address
                               // network byte ordered
    uint32_t sin6_scope_id;    // set of interfaces for a scope
}
```

- 如果系统支持套接字地址结构中的长度字段，那么 `SIN6_LEN` 常值必须定义。
- IPv6的地址族是 `AF_INET6`，IPv4的地址族是 `AF_INET`。
- 结构中字段的顺序做过编排，使得如果 `sockaddr_in6` 结构本身是64位对其的，那么128位的 `sin6_addr` 字段也是64位对齐的。在一些64位处理机上，如果64位数据存储在某个64位边界地址，那么对它的访问将得到优化处理。
- `sin6_flowinfo` 字段分成两个字段：  
    - 低序20位是流标（flow label）。
    - 高序12位保留。
- 对于具备范围的地址（scoped address），`sin6_scope_id` 字段标识其范围，最常见的是链路局部地址（link-local address）的接口索引（interface index）。

### 3.2.4 新的通用套接字地址结构

不像 `struct sockaddr`，新的 `struct sockaddr_storage` 足以容纳系统所支持的任何套接字地址结构。`sockaddr_storage` 结构在 `netinet/in.h` 头文件中定义，如下所示。

```c
struct sockaddr_storage
{
    uint8_t ss_len;        // length of this struct(implementation dependent)
    sa_family_t ss_family; // address family: AF_xxx value
}
```

- 如果系统支持的任何套接字地址结构有对齐需要，那么 `sockaddr_storage` 能满足最苛刻的对齐要求。
- `sockaddr_storage` 足够大，能够容纳系统支持的任何套接字地址结构。

注意，除了 `ss_family` 和 `ss_len` 外（如果有的话），`sockaddr_storage` 中的其他字段对于用户来说是透明的。`sockaddr_storage` 结构必须强制类型转换成或复制到适合于 `ss_family` 字段所给出地址类型的套接字地址结构中，才能访问其他字段。

## 3.3 值-结果参数

我们之前提到过，当往一个套接字函数传递一个套接字地址结构时，传递的是指向该结构的一个指针。该结构的长度也作为一个参数来传递，不过其传递方式取决于该结构的传递方向：从进程到内核，还是从内核到进程。

1. 从进程到内核传递套接字地址结构的函数有3个：`bind`、`connect` 和 `sendto`。这些函数的参数一个是指向某个套接字地址结构的指针，另一个参数是该结构的整数大小，例如：

```c
struct sockaddr_in serv;

// 填充 serv 结构体

connect(sockfd, (struct sockaddr*)&serv, sizeof(serv));
```

既然指针和指针所至内容的大小都传给了内核，内核就知道到底需要从进程复制多少数据进来。

2. 从内核到进程传递套接字地址结构的函数有4个：`accept`、`recvfrom`、`getsockname` 和 `getpeername`。这四个函数的其中两个参数是指向某个套接字地址结构的指针和指向表示该结构大小的整数变量的指针。

```c
struct sockaddr_un cli; // Unix domain
socklen_t len;

len = sizeof(cli);
getpeername(unixfd, (struct sockaddr*)&cli, &len);
// len may have changed
```

把套接字地址结构的大小这个参数从一个整数改为指向某个整数变量的指针，其原因在于：当函数被调用时，结构大小是一个值，它告诉内核该结构的大小，这样内核在写结构时不至于越界。当函数返回时，结构大小又是一个结果，它告诉进程内核在该结构中到底存储了多少信息（有一些套接字的地址结构长度是可变的，比如说Unix域的 `sockaddr_un` ）。这种类型的参数称为 **值-结果（value-result）** 参数。

当使用值-结果参数作为套接字地址结构的长度时，如果套接字地址结构是固定长度的（见图3-6），那么从内核返回的值总是那个固定长度，例如IPv4的 *sockaddr_in* 长度是16，IPv6的 *sockaddr_in6* 长度是28。然而对于可变长度的套接字地址结构（例如Unix域的 *sockaddr_in*），返回值可能小于该结构的最大长度。

## 3.4 字节排序函数

网络字节序和主机字节序之间的转换使用以下4个函数。

```c
#include <netinet/in.h>
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

*h* 代表 *host*，*n* 代表 *network*，*s* 代表 *short*，*l* 代表 *long* 。我们应该把 *s* 视为一个 16 位的值，把 *l* 视为一个32位的值。***注意，在64位的机器上，即使长整数占用64位，但 `htonl` 和 `ntohl` 操作的仍然是32位的值。*** 在那些大端字节序的系统中，这四个函数通常被定义为空宏。

## 3.5 字节操纵函数

操纵多字节字段的函数有两组，它们既不对数据做解释，也不假设数据是以空字符结束的C字符串。当处理套接字地址结构时，我们需要这样的函数，因为诸如IP地址这样的字段，可能包含值为0的字节，但并不是C字符串。以空字符结尾的C字符串是由在 `<string.h>` 头文件中定义、名字以 *str*（表示字符串） 开头的函数处理的。  
名字以 *b*（表示字节）开头的函数源自Berkeley，几乎所有的现今支持套接字函数的系统仍然提供它们。  
名字以 *mem*（表示内存） 开头的函数起源于ANSI C标准，支持ANSI C函数库的系统都提供它们。

以下为源自 Berkeley 的函数。

```c
#include <strings.h>
void bzero(void *s, size_t n);
void bcopy(const void *src, void *dest, size_t n);
int bcmp(const void *s1, const void *s2, size_t n); // 返回：相等则为0，否则非0
```

`bzero` 把目标字符串中指定数目的字节置为0。常用该函数把套接字地址结构初始化为0。`bcopy` 将指定数目的字节从原字符串移动到目标字符串。`bcmp` 比较两个任意的字节串，若相同则返回0，否则非0。

以下为ANSI C函数。

```c
#include <string.h> // 注意，不同于以上，该头文件中没有s
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
```

`memset` 将目标字符串指定数目的字节置为值 *c* 。`memcpy` 类似 `bcopy`，不过两个指针参数的顺序是相反的。当源字符串与目标字符串重叠时，`bcopy` 能够正确处理，`memcpy` 的结果却不可知。这种情况下必须改用ANSI C的 `memmove` 函数。`memcmp` 比较两个任意的字符串，相同则返回0，不同则返回非0，是大于0还是小于0取决于第一个不等的字节。若 *s1* 对应的字节 > *s2* 对应的字节，则返回值大于0，否则小于0。比较的操作是在假设两个不等的字节均为无符号字符的前提下完成的。

> 小提示：记住 `memcpy` 两个指针参数顺序的方法之一是记着它们是按照与C中的赋值语句相同的顺序从左到右书写的：`dest = src;` `bcopy` 与之相反。

## 3.6 `inet_aton`、`inet_addr` 和 `inet_ntoa` 函数

在本节和下一节我们，我们介绍两组地址转换函数。它们在ASCII字符串（人类易读的形式）与网络字节序的二进制值（这是存放在套接字地址结构中的值）之间转换网际地址。

```c
#include <arpa/inet.h>
int inet_aton(const char *cp, struct in_addr *inp); // 返回：若字符串有效则为1，否则为0
in_addr_t inet_addr(const char *cp); // 已被废弃，不推荐用。
                // 返回：若字符串有效则为32位二进制网络字节序的IPv4地址，否则为INADDR_NONE
char *inet_ntoa(struct in_addr in); // 返回一个指向点分十进制数串的指针
```

- `inet_aton` 函数将 *cp* 所指字符串转换成一个32位的网络字节序二进制，并通过指针 *inp* 来存储。若成功则返回1，否则返回0。该函数还有一个没有被写入正式文档中的特性：如果 *inp* 指针为空，那么该函数仍然对输入的字符串进行有效性检查，但是不存储任何结果。

- `inet_addr` 进行相同的转换，返回值为32位的网络字节序二进制值。该函数存在一个问题：当出错时该函数返回 `INADDR_NONE` 常值（通常是一个32位均为1的值）。这意味着点分十进制数串 *255.255.255.255*（这是IPv4的有限广播地址，见20.2节）不能由该函数处理，因为它的二进制值被用来指示该函数失败。还有一个问题，一些手册页声明该函数出错时返回-1而不是 `INADDR_NONE`，这样在对该函数的返回值（一个无符号的值）和一个负常值（-1）进行比较时可能会发生问题，具体取决于编译器。  
如今 `inet_addr` 已被弃用，新的代码应该改用 `inet_aton` 函数。更好的办法是使用下一节中介绍的函数。

- `inet_ntoa` 函数将一个32位网络字节序二进制IPv4地址转换成相应的点分十进制数串。由该函数的返回值所指向的字符串驻留在静态内存中。这意味着该函数是不可重入的（这个概念见11.18节，该函数的使用见 [TCP/IP网络编程笔记](https://github.com/lixichongAAA/TCP-IP-Network-Programming/tree/master/ch03-%E5%9C%B0%E5%9D%80%E6%97%8F%E4%B8%8E%E6%95%B0%E6%8D%AE%E5%BA%8F%E5%88%97#2-%E5%B0%86%E7%BD%91%E7%BB%9C%E5%AD%97%E8%8A%82%E5%BA%8F%E7%9A%84%E6%95%B4%E6%95%B0%E5%9E%8B%E8%BD%AC%E6%8D%A2%E4%B8%BA%E5%AD%97%E7%AC%A6%E4%B8%B2)）。最后需要注意的是，该函数以一个结构而不是指向该结构的指针作为其参数，这很罕见。

## 3.7 `inet_pton` 和 `inet_ntop` 函数

这两个函数是随IPv6出现的函数，对于IPv4和IPv6地址都适用。函数名中 *p* 和 *n* 分别代表 *表达（presentation）*（也就是ASCII字符串）和 *数值（numerical）*（也就是存放到套接字地址结构中的二进制值）。

```c
#include <arpa/inet.h>
int inet_pton(int af, const char *src, void *dst); 
            // 返回：若成功则为1，若输入不是有效的表达格式则为0，若出错则为-1。
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
            // 若成功则返回指向结果的指针，若出错则为NULL。
```

这两个参数的 *af* 参数既可以是 `AF_INET`，也可以是 `AF_INET6`。如果以不被支持的地址族作为 *af* 参数，这两个参数都返回一个错误，并将 `errno` 置为 `EAFNOSUPPORT`。  
`inet_pton` 尝试转换由 *src* 指针所指的字符串，并通过 *dst* 指针存放二进制结果。`inet_ntop` 进行相反的转换，*size* 参数指定目标存储单元的大小，为有助于指定这个大小，`<netinet/in.h>` 头文件中有如下定义：

```c
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46
```

如果 *size* 参数太小，不足以容纳表达格式（包括结尾的空字符），那么返回一个空指针。并置 `errno` 为 `ENOSPC`。`inet_ntop` 的 *dst* 参数不可以是空指针。调用者必须为目标单元分配内存并指定其大小。调用成功时，这个指针就是该函数的返回值。

## 习题

`inet_aton` 和 `inet_addr` 函数对于接受什么作为点分十进制IPv4数串一直十分随意：允许用小数点分隔的1\~4个数，也允许由一个前导的0x来指定一个十六进制数，还允许一个前导的0来指定一个八进制数。`inet_pton` 函数对IPv4地址的要求却严格的多，明确要求用三个小数点来分隔四个在0\~255之间的数。当指定地址族为 `AF_INET6` 时，`inet_pton` 不允许指定点分十进制数地址，不过有可能有人争辩说应该允许，返回值就是这个点分十进制数串的IPv4映射的IPv6地址（见图A-10）。