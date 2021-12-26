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
 
//��ӡ16���ƺ�ascii
void print(u_char*payload,int len,int offset,int maxlen)
{
	printf("%.5d  ",offset);	//��ӡƫ����(���Ϊ5)
	int max=maxlen;	//���ݰ�����Ч�غɺͳ���
	int i;
	for(i=0;i<16;i++)	//��ӡ16���ֽڵ�16����payload
	{
		if((len-i)>0)	//��û����
		{
		printf("%.2x ",payload[max-(len-i)]);
		}
		else	//�Ѵ��꣬���һ�������пո�
		{
		printf("   ");
		}
	}
	printf("       ");
	for(i=0;i<16;i++)	//��ӡ16���ֽڵ�asciipayload
	{
		if(isprint(payload[max-(len-i)]))	//Ϊ�ɴ�ӡ�ַ�
		{
			printf("%c",payload[max-(len-i)]);
		}
		else		//��ӡ����������"."��ʾ
		{
			printf(".");
		}
	}
}
 
//��ӡ���ݰ�
void print_data(u_char *payload,int len)
{
	int line_width=16;	//һ��16���ֽ�
	int len_rem=len;	//ʣ�೤��
	int maxlen=len;		//���ݰ�����Ч�غɺͳ���
	int offset=0;		//ƫ����
	while(1)
	{
		if(len_rem<line_width)	//���һ�δ�ӡ
		{
			if(len_rem==0)	//�Ѵ�ӡ��
				break;
			else {	//��û��ӡ��
			print(payload,len_rem,offset,maxlen);	//����print����������payload��ַ��ʣ�೤�ȡ�ƫ���������ݰ�����Ч�غɺͳ���
			offset=offset+len_rem;	//ƫ��������
			printf("\n");
			break;
			}}
		else	//�������һ�δ�ӡ
		{	
			print(payload,len_rem,offset,maxlen);	//����print����������payload��ַ��ʣ�೤�ȡ�ƫ���������ݰ�����Ч�غɺͳ���
			offset=offset+16;	//ƫ��������(���ڷ����һ�δ�ӡ�����Թ̶���16���ֽ� - ƫ��������16���ֽ�)
			printf("\n");
		}
		len_rem=len_rem-line_width;	//ʣ�೤�ȼ���
	}
}
 
//��ӡmac��ַ
void print_mac(u_char* macadd){
	int i;
	for(i=0;i<5;i++){
		printf("%.2x:",macadd[i]);	//16���ƣ���λ���
		}
	printf("%.2x",macadd[i]);
}
 
//��ӡip��ַ	
void print_ip(u_char* ipadd){
	int i;
	for(i=0;i<3;++i)
		{
		printf("%d.",ipadd[i]);
		}
	printf("%d",ipadd[i]);
}
pcap_handler callback(u_char *user,const struct pcap_pkthdr *h,const u_char *p)//һ����pcap_dispatch()�������ݹ�����u_charָ�룬һ��pcap_pkthdr�ṹ��ָ�룬��һ�����ݰ���С��u_charָ�롣
{
    struct ether_header *eth;	//��̫��֡ͷ��
	static long int packet_num=0;	//��ǰ�����(������)��Ϊ��̬����
	struct ether_arp *arppkt;	//arp֡ͷ��(��arp����Դ��̫��ַ��RTSG, Ŀ���ַ��ȫ��̫����)
	struct ip *iph;	//IP��ͷ��
	struct icmphdr *icmp;	//ICMP��ͷ��
	struct tcphdr *tcph;	//tcpͷ��
	struct udphdr *udph;	//udpͷ��
	int m;
	char *buf;
 
	printf("...............................................................................\n");
	printf("\n");	
	printf("Recieved at ----- %s",ctime((const time_t*)&(h->ts).tv_sec));	//��ʾʱ��
	printf("Packet  number:%d\n",++packet_num);	//��ʾ��ǰ�����
	printf("Packet  length:%d\n",h->len);	//��ʾ������(�ѻ�����)
	int i;
	eth=(struct ether_header *)p;
	printf("Source Mac Address: ");	
	print_mac(eth->ether_shost);	//����print_mac����������Դ������mac��ַ
	printf("\n");
	printf("Destination Mac Address:");
	print_mac(eth->ether_dhost);	//����print_mac����������Ŀ��������mac��ַ
	printf("\n");
	
	//�ж������Э��
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
	if(typeno==ETHERTYPE_IP)	//ΪIPЭ��
		{
			iph=(struct ip*)(p+sizeof(struct ether_header));	//���ip��ͷ����ַ
			
			printf("Source Ip Address:");
			print_ip((u_char*)&(iph->ip_src));	//����print_ip����������Դ������ip��ַ
			printf("\n");
			
			printf("Destination Ip address:");
			print_ip((u_char *)&(iph->ip_dst));	//����print_ip����������Ŀ��������ip��ַ
			printf("\n");
 
			//�жϴ����Э��
			printf("Transport layer protocal:");
			if(iph->ip_p==1)
			{
				printf("ICMP\n");
			}
			else if(iph->ip_p==2)
			{
				printf("IGMP\n");
			}		
			else if(iph->ip_p==6)	//ΪTCPЭ��
			{
				printf("TCP\n");
				tcph=(struct tcphdr*)(p+sizeof(struct ether_header)+sizeof(struct ip));	//���tcpͷ����ַ
				printf("destport :%d\n",ntohs(tcph->dest));	//��ӡĿ�Ķ˿ں�
				printf("sourport:%d\n",ntohs(tcph->source));	//��ӡԴ�˿ں�
				printf("Payload");
				printf("(%d bytes): \n",h->len);	
				print_data(p,h->len);
			}
			else if(iph->ip_p==17)	//ΪUDPЭ��
			{
				printf("UDP\n");
				udph=(struct udphdr*)(p+sizeof(struct ether_header)+sizeof(struct ip));	//���udpͷ����ַ
				printf("dest port:%d\n",ntohs(udph->dest));	//��ӡĿ�Ķ˿ں�
				printf("source port:%d\n",ntohs(udph->source));	//��ӡԴ�˿ں�
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
	char *net_c; //�ַ�����ʽ�����ַ�����ڴ�ӡ���
	char *mask_c; //�ַ�����ʽ�����������ַ
	char errbuf[PCAP_ERRBUF_SIZE];
	struct in_addr addr;
	struct pcap_pkthdr header;//libpcap��ͷ�ṹ����������ʱ�䣬���񳤶������ݰ�ʵ�ʳ���
	const u_char *packet;//���񵽵�ʵ�����ݰ�����
	pcap_t *handle;//libpcap�豸��������
 
	struct bpf_program fp;//������
	char filter_exp[] = "tcp port 80";//ʵ�ʵĹ��˹���
	 	
	struct tm *now_tm;
	time_t now;
	
	bpf_u_int32 net = 0;
	bpf_u_int32 mask = 0;
 
	dev = NULL;
	memset(errbuf,0,PCAP_ERRBUF_SIZE);
 
	//pcap_lookupdev �����豸���� 
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL) {
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return(2);
	}
	printf("Device: %s\n", dev);
	//lookupnet���ָ�������豸������ź����� 
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Can't get netmask for device %s\n", dev);
		net_c = 0;
		mask_c = 0;
		return(2);
 	}
 	//ת�������ַ 
	addr.s_addr = net;
	net_c = inet_ntoa(addr);
	printf("Net: %s\n", net_c);
 
	addr.s_addr = mask;
	mask_c = inet_ntoa(addr);
	printf("Mask: %s\n",mask_c);
 
	printf("==================================================\n");
	//pcap_open_live���豸�ļ�׼����ȡ���� 
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return(2);
	}
	//������˹��� 
	if (pcap_compile(handle, &fp, "ip", 1, mask) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	//���ù��˹��� 
	if (pcap_setfilter(handle, &fp) == -1) {
	 	fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
	 	return(2);
 	}
 	//�������ݰ�
 	//pcap_loopѭ��ץȡ�������ݱ��� ���ûص�����ʵ�� 
	if(pcap_loop(handle,-1,callback,NULL)<0)
	{
		(void)fprintf(stderr,"pcap_loop:%s\n",pcap_geterr(handle));
		exit(0);
	}
	//�رտ� 
	pcap_close(handle);
	return(0);
}
		
