/*	linux-socket-select-�첽������
	talk_server.c
	writed by hanzhongqiu 13/04/2009
	Using select() for I/O multiplexing 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

/* port we're listening on */
#define SERVERPORT 1573
/*maximum cnnnect number*/
#define BACKLOG 10
/*maximum data buffer*/
#define BUFSIZE 2048

struct client
{
	int fd;
	char username[256];
    int first;//初始化为0，输了用户名之后为1 
};

int main()
{
	/*master file descriptor list*/
	fd_set master_fds;
	/*temple file descriptor list for select()*/
	fd_set read_fds;
	/*server address*/
	struct sockaddr_in server_addr;
	/*client address*/
	struct sockaddr_in client_addr;
	/*maximum file descriptor number*/
	int max_fd;
	/*listening socket file descriptor*/
	int sockfd;
	/*newly accept()ed socket file descreptor*/
	int newfd;
	/*buffer for saving client data*/
	char data_buf[BUFSIZE];
	char send_buf[BUFSIZE];
	/*number of client data*/
	int nbytes;
	/*for set socket option*/
	int opt;
	opt = SO_REUSEADDR;
    /*lenth of address*/
	int addr_len;
	/*for accept() to use*/
	int size;
	size = sizeof(struct sockaddr);
	/*temple varient*/
	int tmp_i, tmp_j;
	struct client clientinfo[BACKLOG];
	
	/*clear the master and temple file descriptor*/
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
	
  memset(&data_buf, 0, BUFSIZE);
	memset(&send_buf, 0, BUFSIZE);

	/*create socket*/
	if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("create socket() error:");
		exit(1);
	}
	/*set the socket*/
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
			perror("setsockopt() error:");
			exit(1);
    }
	/*bind first config the socket then binding*/
	memset(&server_addr, 0, size);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(server_addr.sin_zero), 8);
	if (-1 == bind(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
	{
		perror("bind() socket error:");
		exit(1);
	}
	/*listen */
	if (-1 == listen(sockfd, BACKLOG))
	{
		perror("listen() error:");
		exit(1);
	}
	/*add sockfd to master set*/
	FD_SET(sockfd, &master_fds);
	/*keep track the max file descriptor*/
	max_fd = sockfd;
	printf("server is ok!\n");
	/*loop*/
	while (1)
	{
		read_fds = master_fds;
		if (-1== select(max_fd + 1, &read_fds, NULL, NULL, NULL))
		{
			perror("select() error!\n");
			exit(1);
		}
		/*looking for data to read*/
		for (tmp_i = sockfd; tmp_i <= max_fd; tmp_i++)
		{
			/*got connect*/
			if (FD_ISSET(tmp_i, &read_fds))
			{
				if (tmp_i == sockfd)
				{
					newfd = accept(sockfd, (struct sockaddr*)&client_addr, &size);
					clientinfo[newfd].fd = newfd;
					clientinfo[newfd].first=0;
					if (-1 == newfd)
					{
						perror("accept() error:");
						exit(1);
					}
					else
					{
						FD_SET(newfd, &master_fds);
						if (newfd > max_fd)
						{
							max_fd = newfd;
						}
						
					}
				}
				else
				{/*get data from the client*/
							nbytes = read(tmp_i, data_buf, sizeof(data_buf));
                            //data_buf[strlen(data_buf)-1]='\0';
							if (0 > nbytes)
							{
								perror("recv() error:");
								exit(1);
							}
							else if(clientinfo[tmp_i].first==0)
							{	
								data_buf[strlen(data_buf)-1]='\0';
                                clientinfo[tmp_i].first=1;
                                strcpy(clientinfo[tmp_i].username,data_buf);
                                
								printf("%s come!\n",data_buf);
								strcat(send_buf, " welcome\n");
                                if (-1 == write(tmp_i, send_buf, strlen(send_buf)))
								{
									perror("send data error:");
								}
								
							}
							else
							{
								printf("get data:%s from the client :", data_buf);
								
                                if(data_buf[0]=='/'){
                                    
                                    //执行相关命令 /send /who /help /quit
                                    if(0==strncmp(&data_buf[1],"send",4)){
                                        //just send to sb
                                        char  *p =strtok(data_buf," ");
                                        p=strtok(NULL, " ");
                                        
                                        for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++){
                                            if (FD_ISSET(tmp_j, &master_fds)){
                                                if(0==strcmp(p,clientinfo[tmp_j].username)){
                                                    p=strtok(NULL, " ");
                                                    strcpy(send_buf,p);
                                                    if (-1 == write(tmp_j, send_buf, strlen(send_buf)))
										            {
										    	        perror("send data error:");
										            }
                                                    break;
                                                }
                                            }
                                        }
                                            


                                    }else if (0==strncmp(&data_buf[1],"who",3)){
                                        
                                        for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++){
                                            if (FD_ISSET(tmp_j, &master_fds))
									        {

										        strcat(send_buf,clientinfo[tmp_j].username);
                                                strcat(send_buf,"\n");
									        }
                                        }
                                        
                                        if (-1 == write(tmp_i, send_buf, strlen(send_buf)))
										{
										    	perror("send data error:");
										}
                                    }else if (0==strncmp(&data_buf[1],"help",4)){
                                        strcat(send_buf,"/help return the help information\n");
                                        strcat(send_buf,"/send only send to someone\n");
                                        strcat(send_buf,"/who all users ' name in the chatroom\n");
                                        strcat(send_buf,"/quit just exit from the chatroom\n");
                                        if (-1 == write(tmp_i, send_buf, strlen(send_buf)))
										{
										    	perror("send data error:");
										}
                                    }else if (0==strncmp(&data_buf[1],"quit",4)){
                                        printf("client: %s exit!\n", clientinfo[tmp_i].username);
								        FD_CLR(tmp_i, &master_fds);
								        close(tmp_i);
								        strcat(send_buf, clientinfo[tmp_i].username);
								        strcat(send_buf, "  was exit!");
								        for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
								        {
									        if (FD_ISSET(tmp_j, &master_fds))
									        {

										        if (-1 == write(tmp_j, send_buf, strlen(send_buf)))
										        {
											        perror("send data error:");
										        }
									        }
                                        }
								    }// end for
                                    else{
                                        strcat(send_buf,"command error");
                                        if (-1 == write(tmp_j, send_buf, strlen(send_buf)))
										{
										    	perror("send data error:");
										}
                                    }
                                }else{
                                    //广播相关话
                                    strcat(send_buf, clientinfo[tmp_i].username);
                                    strcat(send_buf, " said:  ");
								    strcat(send_buf, data_buf);
                                    for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
								    {
									    if (FD_ISSET(tmp_j, &master_fds))
									    {

										    if (-1 == write(tmp_j, send_buf, strlen(send_buf)))
										    {
										    	perror("send data error:");
										    }
									    }
								    }// end for
                                }
								
							}//end else
					
					
					
				}//end else
			}//end if
		}//end for 

    memset(&data_buf, 0, BUFSIZE);
	memset(&send_buf, 0, BUFSIZE);
	//FD_ZERO(&master_fds);
	//FD_SET(sockfd, &master_fds);
	}//end while

return 0;
}

