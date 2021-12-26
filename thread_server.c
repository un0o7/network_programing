#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8900
int execute(char* command,char* buf)
{
	FILE * 	fp;
	int count;

	if (NULL==(fp = popen(command,"r")))
	{
		perror("creating pipe error\n");
		exit(1);
	}
	count = 0 ;
	while(((buf[count] = fgetc(fp))!=EOF)&&count<2047)
	count++;
	buf[count]='\0';
	pclose(fp);
	return count;

}

void *client_fun(void *arg)
{
	int recv_len = 0;
	char recv_buf[2048] ;	// 接收缓冲区
    char send_buf[2048];
	int connfd = (int)arg; // 传过来的已连接套接字
    int sendnum;
	// 接收数据
	while((recv_len = recv(connfd, recv_buf, sizeof(recv_buf), 0)) > 0)
	{

        recv_buf[recv_len]='\0';
		printf("received command : %s\n", recv_buf); // 打印数据
        if(0==strcmp(recv_buf,"quit")){
            close(connfd);
            pthread_exit(0);
        }
		sendnum=execute(recv_buf,send_buf);
		send_buf[sendnum]='\0';
		if(sendnum==0){
			sprintf(send_buf,"error command\n");
		}
        		
        send(connfd, send_buf, sendnum, 0);
        memset(send_buf, 0, 2048);
        memset(recv_buf, 0, 2048);
	}
	
	printf("client closed!\n");
	close(connfd);	//关闭已连接套接字
	
	return 	NULL;
}


int main(int argc, char **argv){
    struct sockaddr_in server;
    int port;
    int listend;
    int connectd;
    int opt;
    pthread_t thread_id;

    //if (2==argc)

    port = PORT;

    opt = SO_REUSEADDR;

    if (-1 == (listend = socket(AF_INET, SOCK_STREAM, 0)))//创建套接字
    {
        perror("create listen socket error\n");
        exit(1);
    }
    setsockopt(listend, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//


    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (-1 == bind(listend, (struct sockaddr *)&server, sizeof(struct sockaddr)))//绑定
    {
        perror("bind error\n");
        exit(1);
    }

    if (-1 == listen(listend, 5))//
    {
        perror("listen error\n");
        exit(1);
    }
    while(1){
        struct sockaddr_in client;
        int len=sizeof(client);
        if (-1 == (connectd = accept(listend, (struct sockaddr *)&client, &len)))//建立tcp三次握手
        {
            perror("create connect socket error\n");
            continue;
        }
		pthread_create(&thread_id, NULL, (void *)client_fun, (void *)connectd);  //创建线程
        pthread_detach(thread_id);
    }
    close(listend);
    exit(0);

}
