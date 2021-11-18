#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#define SERVER_PORT 8888
#define BUFF_LEN 512
#define MULTICAST "239.255.42.44"

int AddSocketToMulticast(const int sockfd, const char *multicast)
{
	int ret;

	//set multicast address 
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = inet_addr(multicast);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//add multicast group
	ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if(ret < 0)
	{
		printf("set multicast error \n");
		printf("errno is %d \n", errno);
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int SetMulticastLoop(const int sockfd)
{
	int loop = 1; //1: on , 0: off
	int ret = -1;
	ret = setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop));
	if(ret < 0)
	{
		printf("set multicast loop failed !! \n");
		perror("setsockopt");
		return -1;
	}
	return ret;
}

void udp_msg_sender(int fd, struct sockaddr* dst)
{

    socklen_t len;
    struct sockaddr_in src;
    while(1)
    {
        char buf[BUFF_LEN] = "TEST-2 UDP MSG!\n";
        len = sizeof(*dst);
        printf("client:%s\n",buf);  //打印自己发送的信息
        sendto(fd, buf, BUFF_LEN, 0, dst, len);
        memset(buf, 0, BUFF_LEN);
        recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&src, &len);  //接收来自server的信息
        printf("server:%s\n",buf);
        sleep(1);  //一秒发送一次消息
    }
}

/*
    client:
            socket-->sendto-->revcfrom-->close
*/

int main(int argc, char* argv[])
{
    int client_fd;
    struct sockaddr_in ser_addr;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(MULTICAST); 
    //ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //注意网络序转换,INADDR_ANY：任意地址 0.0.0.0
    ser_addr.sin_port = htons(SERVER_PORT);  //注意网络序转换

	//set multicast loop
	//SetMulticastLoop(client_fd);

	//set socket to add multicast address
	//AddSocketToMulticast(client_fd, MULTICAST);

    udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

    close(client_fd);

    return 0;
}