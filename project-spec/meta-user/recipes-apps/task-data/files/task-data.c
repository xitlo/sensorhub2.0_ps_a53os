/*
 * data_task.c
 *
 *  Created on: Dec 15, 2020
 *      Author: ipc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define IP			"192.168.2.1"
#define PORT_RECV	6000
#define PORT_SEND	7000

typedef struct data {
	char name[30];
	unsigned int num;
}Data;

void print_err(char *str, int line, int err_no) {
	printf("%d, %s :%s\n",line,str,strerror(err_no));
	_exit(-1);
}

int cfd = -1;
//接收线程函数
void *receive(void *pth_arg) {
	int ret = 0;
	Data stu_data = {0};
	struct sockaddr_in addr0 = {0};
	int addr0_size = sizeof(addr0);
	//从对端ip和端口号中接收消息，指定addr0用于存放消息
	while(1) {
		bzero(&stu_data, sizeof(stu_data));
		ret = recvfrom(cfd, &stu_data, sizeof(stu_data),0, (struct sockaddr *)&addr0, &addr0_size);
		if (-1 == ret) {
			print_err("recv failed",__LINE__,errno);
		}
		else if (ret > 0){
			printf("student number = %d student name = %s \n",ntohl(stu_data.num),stu_data.name);
			//打印对方的消息和端口号
			printf("ip %s,port %d\n",\
			inet_ntoa(addr0.sin_addr),ntohs(addr0.sin_port));
		}
	}
}

int main()
{
	int ret = -1;
	//创建tcp/ip协议族，指定通信方式为无链接不可靠的通信
	cfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == cfd) {
		print_err("socket failed", __LINE__, errno);
	}

	//进行端口号和ip的绑定
	struct sockaddr_in addr;
	addr.sin_family = AF_INET; //设置tcp协议族
	addr.sin_port = htons(PORT_RECV); //设置端口号
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //设置ip地址
	ret = bind(cfd, (struct sockaddr*)&addr, sizeof(addr));

	if ( -1 == ret) {
		print_err("bind failed",__LINE__,errno);
	}

	//创建线程函数，用于处理数据接收
	pthread_t id;
	ret = pthread_create(&id,NULL,receive,NULL);
	if (-1 == ret) print_err("pthread_create failed", __LINE__, errno);

	struct sockaddr_in addr0;
	addr0.sin_family = AF_INET; //设置tcp协议族
	addr0.sin_port = htons(PORT_SEND); //设置端口号
	addr0.sin_addr.s_addr = inet_addr(IP); //设置ip地址

	char *str_def = "hello world!";
	Data std_data = {0};
	if( 0 >= snprintf(std_data.name, sizeof(std_data.name), "%s", str_def) ) {
		print_err("snprintf failed",__LINE__,errno);
	}
	std_data.num = strlen(std_data.name);

	//发送消息
	while (1) {
		// bzero(&std_data, sizeof(std_data));
		printf("stu name: %s\n", std_data.name);
		// scanf("%s",std_data.name);


		printf("stu num: %d\n", std_data.num);
		// scanf("%d",&std_data.num);
		// std_data.num = htonl(std_data.num);

		//发送消息时需要绑定对方的ip和端口号
		ret = sendto(cfd, (void *)&std_data,sizeof(std_data), 0, (struct sockaddr *)&addr0, sizeof(addr0));
		if ( -1 == ret) {
			print_err("accept failed", __LINE__, errno);
		}
		sleep(1);
	}
	return 0;
}
