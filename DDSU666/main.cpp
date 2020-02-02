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

	// ��ʼ����ws2_32.lib��
#ifdef _WIN32
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);
#endif

	// ������һ��socket
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		printf("create sock error!\n");
		return -1;
	}

	// TCP Serverָ���˿ڲ����÷���˶˿ڵ����ԣ�����(sockaddr*)&saddr

	unsigned short port = 8080; // Ĭ�϶˿ں�
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	sockaddr_in saddr; // �����˿�
	saddr.sin_family = AF_INET; // TCPIPЭ��
	saddr.sin_port = htons(port); // �󶨶˿ں�, htons()֮host-to-network
	saddr.sin_addr.s_addr = 0; //��htonl(0) ���������ܵ�IP��ַ 0��ʾ��������������IP

	// �󶨶˿ڵ�ָ����socket������(sockaddr*)&saddr
	if (::bind(sock, (sockaddr*)& saddr, sizeof(saddr)) != 0) {
		printf("OS bind socks to this port %d failed\r\n", port);
		return -2;
	}
	printf("OS bind this port %d to sockets successfully!\r\n", port);
	listen(sock, 10); // �����û����Ӻ������ͻ�socket��һ���ͻ�һ��socket����������������еĳ��ȣ���

	for (;;) { // ֧�ֶ���ͻ��˵ڶ���socket
		sockaddr_in caddr; // �ṹ�壺�洢�ͻ��˵������Ϣ:�˿ںź�IP��ַs
		socklen_t len = sizeof(caddr);
		int clientSocket = accept(sock, (sockaddr*)& caddr, &len); // �ڶ���socket: ����һ��socketר�Ŷ�ȡ������clients�����ﻺ������С������listen������ʾΪ10��
		if (clientSocket <= 0) {
			break;
		}
		printf("accept client %d\r\n", clientSocket);
		char* ip = inet_ntoa(caddr.sin_addr); // �ͻ���IP��ַת�ַ���
		unsigned short cport = ntohs(caddr.sin_port);// �ͻ��˶˿ںţ������ֽ���ת�����ֽ���
		printf("client ip: %s, port is %d\n", ip, cport); // ��ӡ�ͻ���������Ϣ

		TcpThread* th = new TcpThread(clientSocket); // �����ڶ���socket����
		thread sth_recv(&TcpThread::TcpRecvThread, th); // �������պ���,����Ϊthread��������Ϊthread sth()
		sth_recv.detach();
		thread sth_send(&TcpThread::TcpSendThread, th); // �������ͺ���,����Ϊthread��������Ϊthread sth()
		sth_send.detach();
	}

#ifdef _WIN32 // �˿ڵĵ�һ��socket�����ٽ�����ҲҪ�ǵùر�,�ȶ���һʱ��ջ˼��
	closesocket(sock);
#else
	close(sock);
#endif

	//getchar();
	return 0;
}
