#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//ϵͳ���ÿ�
#include <unistd.h>
//IO�����⣬ioctl()
#include <sys/ioctl.h>
//�׽��ֿ�
#include <sys/socket.h>
//�׽ӵ�ַ��, sockaddr_in, sockaddr_ll
#include <arpa/inet.h>

//�õ��ĸ����������ݰ��� 
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netpacket/packet.h>

/* ��̫��֡�ײ����� */
#define ETHER_HEADER_LEN sizeof(struct ether_header)
/* ����arp�ṹ���� */
#define ETHER_ARP_LEN sizeof(struct ether_arp)
/* ��̫�� + ����arp�ṹ���� */
#define ETHER_ARP_PACKET_LEN ETHER_HEADER_LEN + ETHER_ARP_LEN
/* IP��ַ���� */
#define IP_ADDR_LEN 4
/* �㲥��ַ */
#define DESTINATION_ADDR {0x00, 0x0c, 0x29, 0xf1, 0xa6, 0x84}

void err_exit(const char *err_msg)
{
    perror(err_msg);
    exit(1);
}

/* ���arp�� */
struct ether_arp *fill_arp_packet(const unsigned char *src_mac_addr, const char *src_ip, const char *dst_ip)
{
    struct ether_arp *arp_packet;
    struct in_addr src_in_addr, dst_in_addr;
    unsigned char dst_mac_addr[ETH_ALEN] = DESTINATION_ADDR;

    /* IP��ַת�� */
    inet_pton(AF_INET, src_ip, &src_in_addr);
    inet_pton(AF_INET, dst_ip, &dst_in_addr);

    /* ����arp�� */
    arp_packet = (struct ether_arp *)malloc(ETHER_ARP_LEN);
    arp_packet->arp_hrd = htons(ARPHRD_ETHER);
    arp_packet->arp_pro = htons(ETHERTYPE_IP);
    arp_packet->arp_hln = ETH_ALEN;
    arp_packet->arp_pln = IP_ADDR_LEN;
    arp_packet->arp_op = htons(ARPOP_REQUEST);
    memcpy(arp_packet->arp_sha, src_mac_addr, ETH_ALEN);
    memcpy(arp_packet->arp_tha, dst_mac_addr, ETH_ALEN);
    memcpy(arp_packet->arp_spa, &src_in_addr, IP_ADDR_LEN);
    memcpy(arp_packet->arp_tpa, &dst_in_addr, IP_ADDR_LEN);

    return arp_packet;
}

/* arp���� */
void arp_request(const char *if_name, const char *dst_ip)
{
    struct sockaddr_ll saddr_ll;
    struct ether_header *eth_header;
    struct ether_arp *arp_packet;
    struct ifreq ifr;
    char buf[ETHER_ARP_PACKET_LEN];
    unsigned char src_mac_addr[ETH_ALEN]={0x12,0x34,0x56,0x78,0x12,0x34};
    unsigned char dst_mac_addr[ETH_ALEN] = DESTINATION_ADDR;
    char *src_ip;
    int sock_raw_fd, ret_len, i;

    if ((sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)
        err_exit("socket()");

    bzero(&saddr_ll, sizeof(struct sockaddr_ll));
    bzero(&ifr, sizeof(struct ifreq));
    /* �����ӿ��� */
    memcpy(ifr.ifr_name, if_name, strlen(if_name));

    /* ��ȡ�����ӿ����� */
    if (ioctl(sock_raw_fd, SIOCGIFINDEX, &ifr) == -1)
        err_exit("ioctl() get ifindex");
    saddr_ll.sll_ifindex = ifr.ifr_ifindex;
    saddr_ll.sll_family = PF_PACKET;

    /* ��ȡ�����ӿ�IP */
  
    src_ip = "192.168.212.2";
    printf("local ip:%s\n", src_ip);
    /* ��ȡ�����ӿ�MAC��ַ */
   

    bzero(buf, ETHER_ARP_PACKET_LEN);
    /* �����̫�ײ� */
    eth_header = (struct ether_header *)buf;
    memcpy(eth_header->ether_shost, src_mac_addr, ETH_ALEN);
    memcpy(eth_header->ether_dhost, dst_mac_addr, ETH_ALEN);
    eth_header->ether_type = htons(ETHERTYPE_ARP);
    /* arp�� */
    arp_packet = fill_arp_packet(src_mac_addr, src_ip, dst_ip);
    memcpy(buf + ETHER_HEADER_LEN, arp_packet, ETHER_ARP_LEN);
	
    /* �������� */
	
	
	while(1)
	{  
		ret_len = sendto(sock_raw_fd, buf, ETHER_ARP_PACKET_LEN, 0, (struct sockaddr *)&saddr_ll, sizeof(struct sockaddr_ll));
		if ( ret_len > 0)
			printf("sendto() ok!!!\n");
		sleep(1);
	}

    close(sock_raw_fd);
   
}
int arp_check(const char *if_name, const char *dst_ip)
{
    struct sockaddr_ll saddr_ll;
    struct ether_header *eth_header;
    struct ether_arp *arp_packet;
    struct ifreq ifr;
    char buf[ETHER_ARP_PACKET_LEN];
    unsigned char src_mac_addr[ETH_ALEN];
    unsigned char dst_mac_addr[ETH_ALEN] = DESTINATION_ADDR;
    char *src_ip;
    int sock_raw_fd, ret_len, i;

    if ((sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)
        err_exit("socket()");

    bzero(&saddr_ll, sizeof(struct sockaddr_ll));
    bzero(&ifr, sizeof(struct ifreq));
    /* �����ӿ��� */
    memcpy(ifr.ifr_name, if_name, strlen(if_name));
    struct timeval timeout={3,0};//3s
    int ret=setsockopt(sock_raw_fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    
    /* ��ȡ�����ӿ����� */
    if (ioctl(sock_raw_fd, SIOCGIFINDEX, &ifr) == -1)
        err_exit("ioctl() get ifindex");
    saddr_ll.sll_ifindex = ifr.ifr_ifindex;
    saddr_ll.sll_family = PF_PACKET;

    /* ��ȡ�����ӿ�IP */
    if (ioctl(sock_raw_fd, SIOCGIFADDR, &ifr) == -1)
        err_exit("ioctl() get ip");
    src_ip = inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr);
    printf("local ip:%s\n", src_ip);

    /* ��ȡ�����ӿ�MAC��ַ */
    if (ioctl(sock_raw_fd, SIOCGIFHWADDR, &ifr))
        err_exit("ioctl() get mac");
    memcpy(src_mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    printf("local mac");
    for (i = 0; i < ETH_ALEN; i++)
        printf(":%02x", src_mac_addr[i]);
    printf("\n");

    bzero(buf, ETHER_ARP_PACKET_LEN);
    /* �����̫�ײ� */
    eth_header = (struct ether_header *)buf;
    memcpy(eth_header->ether_shost, src_mac_addr, ETH_ALEN);
    memcpy(eth_header->ether_dhost, dst_mac_addr, ETH_ALEN);
    eth_header->ether_type = htons(ETHERTYPE_ARP);
    /* arp�� */
    arp_packet = fill_arp_packet(src_mac_addr, src_ip, dst_ip);
    memcpy(buf + ETHER_HEADER_LEN, arp_packet, ETHER_ARP_LEN);
	
    /* �������� */
	
        
		ret_len = sendto(sock_raw_fd, buf, ETHER_ARP_PACKET_LEN, 0, (struct sockaddr *)&saddr_ll, sizeof(struct sockaddr_ll));
		if ( ret_len > 0)
			printf("check sendto() ok!!!\n");
		

         bzero(buf, ETHER_ARP_PACKET_LEN);
         bzero(arp_packet,sizeof(struct ether_arp *));

         ret_len=0;
         ret_len = recv(sock_raw_fd, buf, ETHER_ARP_PACKET_LEN, 0);
         
         if (ret_len > 0)
         {
             /* ��ȥ��̫ͷ�� */
             arp_packet = (struct ether_arp *)(buf + ETHER_HEADER_LEN);
             /* arp������Ϊ2����arpӦ�� */
             if (ntohs(arp_packet->arp_op) == 2)
             {
                 //printf("==========================arp replay======================\n");
                 printf("host:");
                 for (i = 0; i < IP_ADDR_LEN; i++)
                     printf(".%u", arp_packet->arp_spa[i]);
                printf(" active!!!\n");
                 printf("from mac");
                 for (i = 0; i < ETH_ALEN; i++)
                     printf(":%02x", arp_packet->arp_sha[i]);
                 printf("\n");
                 close(sock_raw_fd);
                 return 1;
             }
         }else{
            close(sock_raw_fd);
            printf("closed!\n");
            return 0;
         }
    
}
int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        printf("usage:%s device_name dst_ip\n", argv[0]);
        exit(1);
    }
    int result=arp_check(argv[1], argv[2]);
    if(result==0)return 0;
    printf("check ok\n");
    arp_request(argv[1], argv[2]);
    
    return 0;
}

