#include "TcpThread.h"


TcpThread::TcpThread(int clientSocket)
{
	m_sendTestFlag = 1;
	m_sendFlag = 1;
	m_modbusDevId = 0;
	m_clientSocket = clientSocket;
}
TcpThread::~TcpThread()
{

}
void TcpThread::TcpRecvThread() // ����
{
	static char recvBuf[2048] = { 0 };
	for (;;) {
		int recvLen = recv(m_clientSocket, recvBuf, sizeof(recvBuf), 0); // server��ȡclient�˼��������(�ڶ���socket�ľ�����洢���ݵĵط�,flag)
		if (recvLen <= 0) {
			m_sendFlag = 0;
			printf("[Socket %d] disconnect!\r\n", m_clientSocket);
			break;
		}
		printf("[Socket %d] Recv: ", m_clientSocket); // ��������ʾ�ͻ��˼�����ַ�������
		for (int i = 0; i < recvLen; i++) {
			printf(" %02X", recvBuf[i] & 0xFF);
		}
		printf("\r\n");

		// �ǲ���ģʽ ���� ���ص��豸��ַ���� 
		if ((m_sendTestFlag == 0) && ((recvBuf[0] & 0xFF) != m_modbusDevId)) {
			continue;
		}

		//�����벻��
		if ((recvBuf[1] & 0xFF) != 0x03) {
			continue;
		}

		if ((recvBuf[2] & 0xFF) == 0x22) {
			DataAnalysis1(&(recvBuf[3]));
		}
		else if ((recvBuf[2] & 0xFF) == 0x24) {
			DataAnalysis2(&(recvBuf[3]));
		}
		else if ((recvBuf[2] & 0xFF) == 0x04) {
			DataAnalysis3(&(recvBuf[3]));
		}

	}

#ifdef _WIN32 // ��ȡ���ݵĵڶ���socket������Ҫ�ǵùر�
	closesocket(m_clientSocket);
#else
	close(m_client);
#endif

	//delete this; // ��������Լ�������ڶ���socket�Ķ���
}


void TcpThread::TcpSendThread() // ����
{
	// ��ȡ�豸��ַ
	printf("\r\n------------------------------------\r\n");
	int testModbusDevId = 0x01;
	while (m_sendTestFlag) {
		DataRead1(testModbusDevId++);
		Sleep(500);
		if (testModbusDevId > 0xFF) {
			testModbusDevId = 0x01;
		}
	}

	// ��ȡ����
	while (m_sendFlag) {
		printf("\r\n------------------------------------\r\n");
		DataRead2();
		Sleep(1000);
		DataRead3();
		Sleep(10000);
	}

	delete this; // ��������Լ�������ڶ���socket�Ķ���
}

unsigned int TcpThread::CrcCal(const char* pBuff, int len)
{
	unsigned int mid = 0;
	unsigned char times = 0, Data_index = 0;
	unsigned int  cradta = 0xFFFF;
	while (len)
	{
		cradta = pBuff[Data_index] ^ cradta;//������֡�еĵ�һ���ֽڵ�8λ��CRC�Ĵ����еĵ��ֽڽ���������㣬������CRC�Ĵ���
		for (times = 0; times < 8; times++) {
			mid = cradta;
			cradta = cradta >> 1;
			if (mid & 0x0001) {
				cradta = cradta ^ 0xA001;
			}
		}
		Data_index++;
		len--;
	}
	return cradta;
}

// hexתdec
long TcpThread::HextoDec(const char* hex, int start, int length)
{
	int i;
	int end = start + length;
	long dec = 0;
	for (i = start; i < end; i++) {
		dec = dec << 8;
		dec += hex[i];
	}
	return dec;
}

void TcpThread::DataAnalysis1(const char* pBuff)
{
	unsigned int iUcodE = HextoDec(pBuff, 0, 2);
	printf("UcodE: %d\r\n", iUcodE);
	unsigned int iREV = HextoDec(pBuff, 2, 2);
	printf("REV: %d\r\n", iREV);
	unsigned int iAddr = HextoDec(pBuff, 12, 2);
	printf("Addr: %d\r\n", iAddr);
	m_modbusDevId = iAddr;
	unsigned int iBAud = HextoDec(pBuff, 24, 2);
	printf("BAud: %d\r\n", iBAud);

	m_sendTestFlag = 0;
}
void TcpThread::DataAnalysis2(const char* pBuff)
{
	unsigned int iV = HextoDec(pBuff, 0, 4);
	float* fV = (float*)& iV;
	printf("V: %fV\r\n", *fV);

	unsigned int iI = HextoDec(pBuff, 4, 4);
	float* fI = (float*)& iI;
	printf("I: %fA\r\n", *fI);

	unsigned int iP1 = HextoDec(pBuff, 8, 4);
	unsigned int iP2 = HextoDec(pBuff, 12, 4);
	unsigned int iP3 = HextoDec(pBuff, 16, 4);
	float* fP1 = (float*)& iP1;
	float* fP2 = (float*)& iP2;
	float* fP3 = (float*)& iP3;
	printf("P: %fw %fw %fw\r\n", *fP1, *fP2, *fP3);

	unsigned int iPF = HextoDec(pBuff, 20, 4);
	float* fPF = (float*)& iPF;
	printf("PF: %f\r\n", *fPF);

	unsigned int iF = HextoDec(pBuff, 28, 4);
	float* fF = (float*)& iF;
	printf("F: %fHz\r\n", *fF);
}
void TcpThread::DataAnalysis3(const char* pBuff)
{
	unsigned int iImpEp = HextoDec(pBuff, 0, 4);
	float* fImpEp = (float*)& iImpEp;
	printf("ImpEp: %fKwh\r\n", *fImpEp);
}


void TcpThread::DataRead1(int testModbusDevId)
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = testModbusDevId;
	sendBuf[1] = 0x03;	// ������
	sendBuf[2] = 0x00;	// ��ʼ��ַ��
	sendBuf[3] = 0x00;	// ��ʼ��ַ��
	sendBuf[4] = 0x00;	// ���ݸ�����
	sendBuf[5] = 0x11;	// ���ݸ�����
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// У��λ��
	sendBuf[7] = (crc & 0xFF00) >> 8;	// У��λ��
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		printf("[Socket %d] Send error!\r\n", m_clientSocket);
	}
	else {
		printf("[Socket %d] Send: ", m_clientSocket);
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
	}
}
void TcpThread::DataRead2()
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = m_modbusDevId;
	sendBuf[1] = 0x03;	// ������
	sendBuf[2] = 0x20;	// ��ʼ��ַ��
	sendBuf[3] = 0x00;	// ��ʼ��ַ��
	sendBuf[4] = 0x00;	// ���ݸ�����
	sendBuf[5] = 0x12;	// ���ݸ�����
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// У��λ��
	sendBuf[7] = (crc & 0xFF00) >> 8;	// У��λ��
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		printf("[Socket %d] Send error!\r\n", m_clientSocket);
	}
	else {
		printf("[Socket %d] Send: ", m_clientSocket);
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
	}
}
void TcpThread::DataRead3()
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = m_modbusDevId;
	sendBuf[1] = 0x03;	// ������
	sendBuf[2] = 0x40;	// ��ʼ��ַ��
	sendBuf[3] = 0x00;	// ��ʼ��ַ��
	sendBuf[4] = 0x00;	// ���ݸ�����
	sendBuf[5] = 0x02;	// ���ݸ�����
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// У��λ��
	sendBuf[7] = (crc & 0xFF00) >> 8;	// У��λ��
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		printf("[Socket %d] Send error!\r\n", m_clientSocket);
	}
	else {
		printf("[Socket %d] Send: ", m_clientSocket);
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
	}
}
