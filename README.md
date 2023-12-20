## 地址转换函数
地址的文本表达和存放在套接字中的二进制值之间进行转换  
```
旧：inet_addr(),inet_ntoa()，支持ipv4
新：inet_ntop(),inet_pton()，同时支持ipv4，ipv6
```
# 3.2 套接字地址结构

大多数套接字函数使用一个指向套接字地址的指针作为参数

## ipv4套接字地址结构

sockaddr_in 头文件<netinet/in.h>  
结构体中的 IP 地址以及 port 端口号均以网络字节序进行存储
```cpp
// ipv4 套接字地址结构
// 使用单独的结构体存储 IP 地址有历史原因
struct in_addr{
	in_addr_t s_addr;  // 32 位的 IP 地址（uint32_t）
};

struct sockaddr_in{
	uint8_t        sin_len;      // 套接字地址的长度，并不是所有厂家都支持
	sa_family_t    sin_family;   // AF_INET（unsigned short）
	in_port_t      sin_port;     // 16 位的 ipv4 或 ipv6 端口号（uint16_t）
	struct in_addr sin_addr;     // 32 位 ipv4 地址
	char           sin_zero[8];
}；
```
- sin_len 的增加是为了增加对OSI协议的支持 
- 无须设置和检查 sin_len 字段，除非涉及路由套接字 
- POSIX 只需要三个字段，sin_family, sin_addr, sin_port, 增加 sin_zero 是为了扩充字节 
- IPv4 地址和 TCP，UDP 端口号在套接字地址结构中严格按照字节序进行存储 
- sin_zero 一直初始化为 0，因为在填写地址前我们已经全部初始化为 0 
- 套接字仅在主机上进行使用，在网络中不进行传递，仅传递某些字段 

## 通用套接字地址结构

可以通过其对所有不同地址结构的指针进行处理

```cpp
struct sockaddr{
	uint8_t      sa_len;
	sa_family_t  sa_family;
	char         sa_data[14];
};
```
所以套接字的参数指向某个通用套接字地址  
eg：int bind(int, struct sockaddr*, socklen_t);  
因为当时还不存在 void*，所以需要进行地址结构指针的强制转换  
eg：struct sockaddr_t serv;  
bind (sockfd, (struct sockaddr*)&serv, sizeof(serv));  

## IPv6套接字地址结构

IPV6 套接字地址结构， 头文件<netinet/in.h>  

```cpp
struct in6_addr{
	unit8_t s6_addr[16];  // 128bit IPV6 地址
};
#define SIN6_LEN
struct sockaddr_in6{
	uint8_t         sin6_len;
	sa_family       sin6_family;
	in_port_t       sin6_port;
	uint32_t        sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t        sin6_scope_id;
};
```

- 如果地址支持套接字地址结构中的长度字段，那么 SIN6_LEN 常值必须定义 
- IPv6 的地址族是 AF_INET6，IPv4 的地址族是 AF_INET. 
- 结构体中字段进行过编排，使得 64 位对齐，interesting 
- sin6_flowinfo 字段分为两个字段： 
  - 低序 20 位是流标； 
  - 高序 12 位保留； 
 
## 新的通用套接字地址结构

新的通用套接字地址结构可以容纳所有套接字地址结构。sockaddr_storage结构在<netinet/in.h>头文件中定义 

```cpp
struct sockaddr_storage{
	uint8_t     ss_len;     // length
	sa_family_t ss_family;  // AF_XXX
	// 其余的字段对用户来说是透明的，转换为特定地址结构进行使用
};
```
- 和旧的通用套接字地址相比差异
  - 如果系统支持任何套接字地址结构有对其需要，sockaddr_storage可以满足最苛刻的对其要求 
  - sockaddr_storage足够大，可以满足系统支持的所有套接字地址结构 

#### 除了ss_family和ss_len之外，结构体中其他字段对用户来说是透明的，使用时需要强制转换过去
 
# 3.3 值-结果参数

从进程到内核的传递函数有三个，bind(), connect(), sendto()。  
参数中都有指向某个套接字地址的指针，另一个参数是该结构的整数大小  
```cpp
eg: struct sockaddr_in serv;  
    connect(sockfd, (SA*) &serv, sizeof(serv)); //其中大小参数的类型是socklen_t(uint32_t) 
```
从内核到进程的传递参数有四个accept(), recvfrom(), getsockname(), getpeername()  
这四个函数中有两个函数的参数包括一个指向套接字地址的指针和一个指向套接字大小的指针。 
	
总结part：从进程至内核的传参，传递一个值参数，这样内核进行操作时不至于越界;	  
          当函数返回时，结构大小又是一个结果，它告诉进程内核在结构中存储了多谢好信息，所以需要使用值-结果参数  
**这里虽然使用了值-结果参数，但是对于IPv4和IPv6来说，地址结构总是固定大小，并不需要更改，但是对于Unix域等可变长度结构来说，需要进行改变。**

# 3.4 字节排序函数

大端：将高序字节存储在起始位置	
小端：将低序字节存储在起始位置	
网际协议通过大端进行数据的传送，使用时需要进行判断转换 

```cpp
//大端小端测试函数
int main(int argc, char** argv){
	union{
		short s;
		char c[sizeof(short)];
	} un;
	un.s = 0x0102;
	if (sizeof(short) == 2){
		if(un.c[0] == 1 && un.c[1] == 2){
			printf("big endian\n");
		}
		else if(un.c[0] == 2 && un.c[1] == 1){
			printf("little endian\n");
		}
		else{
			printf("unknow");
		}
	}
	exit(0);
}

```
这里使用四个函数进行转换： 
```cpp
#include<netinet/in.h>
uint16_t htons(uint16_t );
uint32_t hton1(uint32_t );   //均返回，网络字节序的值
uint16_t ntohs(uint16_t );
uint32_t ntoh1(uint32_t );   //均返回，主机字节序的值
```
h -> host, n -> network, s -> short(16), l -> long(32)  
在系统是大端的操作系统中，这四个函数一般为空

# 3.5 字节操纵函数

这里因为一些ip地址中包含0，所以不可以当作普通字符串进行处理 
```cpp
#include<strings.h> 
void bzero(void* dest, size_t nytes) .
void bcopy(const void* src, void* dest, size_t nbytes)
int bcmp(const void* ptrl, const void* ptr2, size_t nbytes)

void *memset(void* desk, int e, size_t len); 
void *memcpy(void* desk, const void* src, size_t nbytes); 
int memcpy(const void* ptrl, const void* ptr2, size_t nbytes); 
```
# 3.6 inet_aton, inet_addr, inet_ntoa 函数

这三个函数在点分十进制数串，和对应的32位的网络二进制间转换IPv4地址	

inet_pton(), inte_ntop()对于 IPv4 和 IPv6 地址均适用	
```cpp
#include<arpa/inet.h>
int inet_aton(const char* strptr, struct in_addr* addr_ptr);   //若字符串有效则为1，否则为0
in_addr_t inet_addr(const char* strptr);     //若字符串有效，则返回32位二进制IPv4地址，否则为INADDR_NONE（通常为32位均为1的值）
char* inet_ntoa(struct in_addr inaddr);      //返回一个指向点分十进制的字符串指针
```

第一个函数将字符串转换为专门存储网络地址结构  
第二个函数将字符串转化为32位二进制IPv4地址（不能处理255.255.255.255的广播地址），现在基本不再使用  
第三个函数将32位网络地址结构转化为点分十进制的字符串指针，这个函数以一个结构体为参数，而不是它的指针

# 3.7 inet_pton 和 inet_ntop函数

这两个函数拓展了对IPv6的支持，现在全部使用这两个函数，上面的可以废弃，记住就对了  
```cpp
#include<arpa/inet.h>
// family 代表转换地址的类型
int inet_pton(int family, const char* strptr, void* addrptr);   //成功返回1，失败返回0，出错返回-1
const char* inet_ntop(int family, const void* addrptr, char* strptr, size_t len);     //成功返回指向字符串的指针，失败返回NULL
```
# 3.8 sock_ntop

inet_ntop() 的一个问题是，他要求调用者必须传递一个指向某个二进制地址的指针，而该地址通常包含在某个套接字地址结构中，这要求调用者需要知道这个结构的格式和地址族
```cpp
//IPv4调用格式
struct sockaddr_in addr;
inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
//IPv6调用格式
struct sockaddr_in6 addr6;
inet_ntop(AF_INET6, &addr6.sin6_addr, str, sizeof(str));
```
以上调用可以看出函数与协议有关，不好不好	
这里UNP自行编写了一个sock_ntop函数用来使其协议无关
```cpp
char* sock_ntop(const struct sockaddr* sa, socklen_t salen){
	char portstr[8];
	static char str[128];
	switch(sa->sa_family){
		case AF_INET: {
			struct sockaddr_in* sin = (struct sokaddr_in *) sa;

			if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
				return (NULL);
			if(ntohs(sin->sin_port) != 0){
				snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
				strcat(str, portstr);
			}
			return(str);
		}
	}
}
```

# 3.9 readn, writen, readline

EINTR错误，表示系统调用被一个捕获的信号中断
``` cpp
//UNP自定义的三个函数
ssize_t readn(int fd, void* vptr, size_t n){
	size_t nleft;
	ssize_t nread;
	char *ptr;
	ptr = vptr;
	nleft = n;
	// 循环读取
	while(nleft > 0){
		if( (nread = read(fd, ptr, nleft)) < 0){
			if(errno == EINTR){
				nread = 0;
			}
			else return -1;
		}
		else if (nread == 0){
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return (n - left);
}

ssize_t writen(int fd, const void* vptr, size_t n){
	size_t nleft;
	ssize_t nwriten;
	const char* ptr;
	ptr = vptr;
	nleft = n;
	// 循环写入
	while(nleft > 0){
		if( (nwriten = write(fd, ptr, nleft)) <= 0){
			if(nwriten < 0 && errno == EINTR){
				nwriten= 0;
			}
			else return -1;
		}
		nleft -= nwriten;
		ptr += nwriten;
	}
	return n;
}

//readline书上提供了两个版本：第一个版本每次使用readn读取一个字节来判断是否遇到'\n'（效率极低）
//                          第二个版本用来建立一个缓冲区来进行提前缓存并进行判断
```

# 小结

come on !!!