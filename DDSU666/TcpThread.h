#pragma once
#include <iostream>
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

class TcpThread { // ���߳���
public:
	TcpThread(int clientSocket);
	~TcpThread();
	void TcpRecvThread(); // ����
	void TcpSendThread(); // ����

private:
	unsigned int CrcCal(const char* pBuff, int Len);
	long HextoDec(const char* hex, int start, int length);
	void DataRead1(int testModbusDevId);
	void DataRead2();
	void DataRead3();
	void DataAnalysis1(const char* pBuff);
	void DataAnalysis2(const char* pBuff);
	void DataAnalysis3(const char* pBuff);

private:
	int m_modbusDevId;	// modbus�豸��ַ
	int m_clientSocket;	// ÿһ���ͻ��˵ĵڶ���socket
	int m_sendTestFlag;
	int m_sendFlag;
};

