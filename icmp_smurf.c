#include <libnet.h> 
#include <stdlib.h>
#include <stdio.h>



// ���ݰ����ͺ���
void send_packets(){
    int i;
    libnet_t *l = NULL;/* libnet��� */   
    libnet_ptag_t protocol_tag;/* Э���� */

    char *payload = "flagg"; /* ���� �������趨up to you*/   
    u_short payload_length = 1000; /* ���س��� */   
    char *device = "ens33";/*�����豸�ӿ�*/   
    char *destination_ip_str = "192.168.118.130";/* Ŀ��IP��ַ�ַ��� */   
    char *source_ip_str = "192.168.118.129"; /* ԴIP��ַ�ַ��� */   


    u_long source_ip = 0; /* ԴIP��ַ */   
    u_long destination_ip = 0; /* Ŀ��IP��ַ */   
    char errbuf[LIBNET_ERRBUF_SIZE]; /* ������Ϣ */   
    int packet_length; /* ���͵����ݰ��ĳ��� */   

    // �������Σ�ICMP��IP�㹹��
    l = libnet_init( /* ��ʼ��libnet */
    LIBNET_RAW4, /* libnet���ͣ�Ϊԭʼ�׽���IPv4���� */
    device,  /* �����豸�ӿ� */errbuf /* ������Ϣ */   
    );   
    
    source_ip = libnet_name2addr4(l, source_ip_str, LIBNET_RESOLVE);   
    /* ��ԴIP��ַ�ַ�����ʽת��Ϊ�����ֽ�˳������� */   
    destination_ip = libnet_name2addr4(l, destination_ip_str, LIBNET_RESOLVE);   
    /* ��Ŀ��IP��ַ�ַ�����ʽת��Ϊ�����ֽ�˳������� */   

    protocol_tag = libnet_build_icmpv4_echo( /* ����ICMP�������ݰ� */    
    ICMP_ECHO, /* ���ͣ���ʱΪ�������� */   
    0,/* ���룬Ӧ��Ϊ0 */   
    0, /* У��ͣ�Ϊ0����ʾ��libnet����Զ����� */   
    123,  /* ��ʶ������ֵΪ123���Լ�������д��ֵ */   
    456, /* ***����ֵΪ245���Լ�������д��ֵ */   
    NULL,  /* ���أ���ֵΪ�� */   
    0, /* ���صĳ��ȣ���ֵΪ0 */   
    l, /* libnet�����Ӧ������libnet_init()�����õ��� */   
    0  /* Э����ǣ���ֵΪ0����ʾ����һ���µ�Э��� */   
    );   

    protocol_tag = libnet_build_ipv4(/* ����IPЭ��� */   
    LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + payload_length,/* IPЭ���ĳ��� */   
    0, /* �������������︳ֵΪ0 */   
    10,  /* ��ʶ�������︳ֵΪ10 */   
    0, /* ƫ�ƣ����︳ֵΪ0 */   
    20,/* ����ʱ�䣬���︳ֵΪ20 */   
    IPPROTO_ICMP,/* �ϲ�Э�����ͣ�������ICMPЭ�� */   
    0, /* У��ͣ�����Ϊ0��ʾ��libnet����У��� */   
    source_ip, /* ԴIP��ַ */   
    destination_ip,/* Ŀ��IP��ַ */   
    NULL, /* ���� */   
    0, /* ���صĳ��� */   
    l,/* libnet��� */   
    0 /* Э����ǣ�Ϊ0��ʾ����һ���µ�IPЭ��� */    //protocol_tag�����������޸�֮ǰ�����ݰ�
    );   

    while(1){
        packet_length = libnet_write(l); /* ������libnet���l��ʾ�����ݰ� */   
        printf("the length of the ICMP packet is %d\n", packet_length);   
        /* ������͵����ݰ���Ϣ */   
        }
    libnet_destroy(l); /* ����libnet */
}

void main()   
{   
   send_packets();
}   