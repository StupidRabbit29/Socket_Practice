#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<iostream>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "10.122.193.42" //��������IP��ַ 
#define PORT 22366			//�˿ں�
#define MSGSIZE 1024		//������ݳ���

bool MyDEBUG = true;

using namespace std;
int main()
{
	//windows socket
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}

	//�����׽���
	SOCKET sClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sClient == INVALID_SOCKET)
	{
		cout << "socket error!" << endl;
		return 0;
	}

	//����IP�Ͷ˿�
	SOCKADDR_IN ServerAddr;
	int AddrLen = sizeof(SOCKADDR_IN);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT);
	ServerAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

	while (true)
	{
		char sendData[MSGSIZE] = { '\0' };
		cin >> sendData;
		sendto(sClient, sendData, sizeof(sendData), 0, (sockaddr *)&ServerAddr, AddrLen);
	
		char recvData[MSGSIZE] = { '\0' };
		recvfrom(sClient, recvData, sizeof(recvData), 0, (sockaddr *)&ServerAddr, &AddrLen);
		cout << recvData << endl;
	}

	closesocket(sClient);
	WSACleanup();
	return 0;
}