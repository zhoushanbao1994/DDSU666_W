#include <stdio.h>
#include <string.h>
#include <thread>
#include <list>
#include "TcpThread.h"

#ifdef _WIN32
#include <windows.h>
#define socklen_t int
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
//#include <thread>
#endif

using namespace std;



int main(int argc, char* argv[]) {

	list<int> clients;

	// 初始化”ws2_32.lib”
#ifdef _WIN32
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);
#endif

	// 创建第一类socket
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		printf("create sock error!\n");
		return -1;
	}

	// TCP Server指定端口并设置服务端端口的属性，返回(sockaddr*)&saddr

	unsigned short port = 8080; // 默认端口号
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	sockaddr_in saddr; // 声明端口
	saddr.sin_family = AF_INET; // TCPIP协议
	saddr.sin_port = htons(port); // 绑定端口号, htons()之host-to-network
	saddr.sin_addr.s_addr = 0; //或htonl(0) 服务器接受的IP地址 0表示接受任意内外网IP

	// 绑定端口到指定的socket，输入(sockaddr*)&saddr
	if (::bind(sock, (sockaddr*)& saddr, sizeof(saddr)) != 0) {
		printf("OS bind socks to this port %d failed\r\n", port);
		return -2;
	}
	printf("OS bind this port %d to sockets successfully!\r\n", port);
	listen(sock, 10); // 允许用户连接函数（客户socket（一个客户一个socket），最大请求数队列的长度，）

	for (;;) { // 支持多个客户端第二类socket
		sockaddr_in caddr; // 结构体：存储客户端的相关信息:端口号和IP地址s
		socklen_t len = sizeof(caddr);
		int clientSocket = accept(sock, (sockaddr*)& caddr, &len); // 第二类socket: 创建一个socket专门读取缓冲区clients（这里缓冲区大小如上行listen代码所示为10）
		if (clientSocket <= 0) {
			break;
		}
		printf("accept client %d\r\n", clientSocket);
		char* ip = inet_ntoa(caddr.sin_addr); // 客户端IP地址转字符串
		unsigned short cport = ntohs(caddr.sin_port);// 客户端端口号（网络字节序转本地字节序）
		printf("client ip: %s, port is %d\n", ip, cport); // 打印客户端连接信息

		TcpThread* th = new TcpThread(clientSocket); // 创建第二类socket对象
		thread sth_recv(&TcpThread::TcpRecvThread, th); // 启动接收函数,参数为thread，函数库为thread sth()
		sth_recv.detach();
		thread sth_send(&TcpThread::TcpSendThread, th); // 启动发送函数,参数为thread，函数库为thread sth()
		sth_send.detach();
	}

#ifdef _WIN32 // 端口的第一类socket，不再交互后也要记得关闭,先二后一时堆栈思想
	closesocket(sock);
#else
	close(sock);
#endif

	//getchar();
	return 0;
}
