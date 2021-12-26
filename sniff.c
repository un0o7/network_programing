#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/in_systm.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include<pcap.h>
#include<netdb.h>
#include<time.h>
#include<sys/time.h>
#include<stdlib.h>
#include<ctype.h>
 
//打印16进制和ascii
void print(u_char*payload,int len,int offset,int maxlen)
{
	printf("%.5d  ",offset);	//打印偏移量(宽度为5)
	int max=maxlen;	//数据包的有效载荷和长度
	int i;
	for(i=0;i<16;i++)	//打印16个字节的16进制payload
	{
		if((len-i)>0)	//还没打完
		{
		printf("%.2x ",payload[max-(len-i)]);
		}
		else	//已打完，最后一个后面有空格
		{
		printf("   ");
		}
	}
	printf("       ");
	for(i=0;i<16;i++)	//打印16个字节的asciipayload
	{
		if(isprint(payload[max-(len-i)]))	//为可打印字符
		{
			printf("%c",payload[max-(len-i)]);
		}
		else		//打印不出来的用"."表示
		{
			printf(".");
		}
	}
}
 
//打印数据包
void print_data(u_char *payload,int len)
{
	int line_width=16;	//一行16个字节
	int len_rem=len;	//剩余长度
	int maxlen=len;		//数据包的有效载荷和长度
	int offset=0;		//偏移量
	while(1)
	{
		if(len_rem<line_width)	//最后一次打印
		{
			if(len_rem==0)	//已打印完
				break;
			else {	//还没打印完
			print(payload,len_rem,offset,maxlen);	//调用print函数，传入payload地址、剩余长度、偏移量和数据包的有效载荷和长度
			offset=offset+len_rem;	//偏移量后移
			printf("\n");
			break;
			}}
		else	//不是最后一次打印
		{	
			print(payload,len_rem,offset,maxlen);	//调用print函数，传入payload地址、剩余长度、偏移量和数据包的有效载荷和长度
			offset=offset+16;	//偏移量后移(由于非最后一次打印，所以固定打16个字节 - 偏移量后移16个字节)
			printf("\n");
		}
		len_rem=len_rem-line_width;	//剩余长度减少
	}
}
 
//打印mac地址
void print_mac(u_char* macadd){
	int i;
	for(i=0;i<5;i++){
		printf("%.2x:",macadd[i]);	//16进制，两位宽度
		}
	printf("%.2x",macadd[i]);
}
 
//打印ip地址	
void print_ip(u_char* ipadd){
	int i;
	for(i=0;i<3;++i)
		{
		printf("%d.",ipadd[i]);
		}
	printf("%d",ipadd[i]);
}
pcap_handler callback(u_char *user,const struct pcap_pkthdr *h,const u_char *p)//一个从pcap_dispatch()函数传递过来的u_char指针，一个pcap_pkthdr结构的指针，和一个数据包大小的u_char指针。
{
    struct ether_header *eth;	//以太网帧头部
	static long int packet_num=0;	//当前包编号(包数量)，为静态分配
	struct ether_arp *arppkt;	//arp帧头部(以arp包的源以太地址是RTSG, 目标地址是全以太网段)
	struct ip *iph;	//IP包头部
	struct icmphdr *icmp;	//ICMP包头部
	struct tcphdr *tcph;	//tcp头部
	struct udphdr *udph;	//udp头部
	int m;
	char *buf;
 
	printf("...............................................................................\n");
	printf("\n");	
	printf("Recieved at ----- %s",ctime((const time_t*)&(h->ts).tv_sec));	//显示时间
	printf("Packet  number:%d\n",++packet_num);	//显示当前包编号
	printf("Packet  length:%d\n",h->len);	//显示包长度(脱机长度)
	int i;
	eth=(struct ether_header *)p;
	printf("Source Mac Address: ");	
	print_mac(eth->ether_shost);	//调用print_mac函数，传入源主机的mac地址
	printf("\n");
	printf("Destination Mac Address:");
	print_mac(eth->ether_dhost);	//调用print_mac函数，传入目的主机的mac地址
	printf("\n");
	
	//判断网络层协议
	unsigned int typeno;
	typeno=ntohs(eth->ether_type);
	printf("network layer protocal:");
	switch(typeno){
	case ETHERTYPE_IP:
		printf("IPV4\n");
		break;
	case ETHERTYPE_PUP:
		printf("PUP\n");
		break;
	case ETHERTYPE_ARP:
		printf("ARP\n");
		break;
	default:
		printf("unknown network layer types\n");
	}	
	if(typeno==ETHERTYPE_IP)	//为IP协议
		{
			iph=(struct ip*)(p+sizeof(struct ether_header));	//获得ip包头部地址
			
			printf("Source Ip Address:");
			print_ip((u_char*)&(iph->ip_src));	//调用print_ip函数，传入源主机的ip地址
			printf("\n");
			
			printf("Destination Ip address:");
			print_ip((u_char *)&(iph->ip_dst));	//调用print_ip函数，传入目的主机的ip地址
			printf("\n");
 
			//判断传输层协议
			printf("Transport layer protocal:");
			if(iph->ip_p==1)
			{
				printf("ICMP\n");
			}
			else if(iph->ip_p==2)
			{
				printf("IGMP\n");
			}		
			else if(iph->ip_p==6)	//为TCP协议
			{
				printf("TCP\n");
				tcph=(struct tcphdr*)(p+sizeof(struct ether_header)+sizeof(struct ip));	//获得tcp头部地址
				printf("destport :%d\n",ntohs(tcph->dest));	//打印目的端口号
				printf("sourport:%d\n",ntohs(tcph->source));	//打印源端口号
				printf("Payload");
				printf("(%d bytes): \n",h->len);	
				print_data(p,h->len);
			}
			else if(iph->ip_p==17)	//为UDP协议
			{
				printf("UDP\n");
				udph=(struct udphdr*)(p+sizeof(struct ether_header)+sizeof(struct ip));	//获得udp头部地址
				printf("dest port:%d\n",ntohs(udph->dest));	//打印目的端口号
				printf("source port:%d\n",ntohs(udph->source));	//打印源端口号
				printf("Payload");
				printf("(%d bytes): \n",h->len);	
				print_data(p,h->len);	
			}
			else 
			{
				printf("unknown protocol\n");
			}
		}
} 
int main(int argc,char** argv)
{
	char * dev;
	char *net_c; //字符串形式网络地址，用于打印输出
	char *mask_c; //字符串形式的网络掩码地址
	char errbuf[PCAP_ERRBUF_SIZE];
	struct in_addr addr;
	struct pcap_pkthdr header;//libpcap包头结构，包含捕获时间，捕获长度与数据包实际长度
	const u_char *packet;//捕获到的实际数据包内容
	pcap_t *handle;//libpcap设备描述符号
 
	struct bpf_program fp;//过滤器
	char filter_exp[] = "tcp port 80";//实际的过滤规则
	 	
	struct tm *now_tm;
	time_t now;
	
	bpf_u_int32 net = 0;
	bpf_u_int32 mask = 0;
 
	dev = NULL;
	memset(errbuf,0,PCAP_ERRBUF_SIZE);
 
	//pcap_lookupdev 返回设备名称 
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL) {
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return(2);
	}
	printf("Device: %s\n", dev);
	//lookupnet获得指定网络设备的网络号和掩码 
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Can't get netmask for device %s\n", dev);
		net_c = 0;
		mask_c = 0;
		return(2);
 	}
 	//转换网络地址 
	addr.s_addr = net;
	net_c = inet_ntoa(addr);
	printf("Net: %s\n", net_c);
 
	addr.s_addr = mask;
	mask_c = inet_ntoa(addr);
	printf("Mask: %s\n",mask_c);
 
	printf("==================================================\n");
	//pcap_open_live打开设备文件准备读取数据 
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return(2);
	}
	//编译过滤规则 
	if (pcap_compile(handle, &fp, "ip", 1, mask) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	//设置过滤规则 
	if (pcap_setfilter(handle, &fp) == -1) {
	 	fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
	 	return(2);
 	}
 	//捕获数据包
 	//pcap_loop循环抓取网络数据报文 采用回调函数实现 
	if(pcap_loop(handle,-1,callback,NULL)<0)
	{
		(void)fprintf(stderr,"pcap_loop:%s\n",pcap_geterr(handle));
		exit(0);
	}
	//关闭库 
	pcap_close(handle);
	return(0);
}
		
