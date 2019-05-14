#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WINSOCK2.H>   
#include <iostream> 
#include <string>

//���������ʹ�õĳ���      
#define SERVER_ADDRESS "127.0.0.1" //��������IP��ַ      
#define PORT           5150         //�������Ķ˿ں�      
#define MSGSIZE        1024         //�շ��������Ĵ�С     

#pragma comment(lib, "ws2_32.lib")      
using namespace std;
int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	while (true) 
	{
		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sclient == INVALID_SOCKET)
		{
			printf("invalid socket!");
			return 0;
		}

		//Զ�̷������ĵ�ַ��Ϣ
		SOCKADDR_IN serAddr;
		memset(&serAddr, 0, sizeof(SOCKADDR_IN));
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(PORT);
		serAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
		if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{  //����ʧ�� 
			printf("connect error !");
			closesocket(sclient);
			return 0;
		}

		string data;
		cin >> data;
		const char * sendData;
		sendData = data.c_str();   //stringתconst char* 
		send(sclient, sendData, strlen(sendData), 0);
		//send()������������ָ����socket�����Է�����
		//int send(int s, const void * msg, int len, unsigned int flags)
		//sΪ�ѽ��������ӵ�socket��msgָ���������ݣ�len��Ϊ���ݳ��ȣ�����flagsһ����0
		//�ɹ��򷵻�ʵ�ʴ��ͳ�ȥ���ַ�����ʧ�ܷ���-1������ԭ�����error 

		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret>0) 
		{
			recData[ret] = 0x00;
			printf(recData);
		}
		closesocket(sclient);
	}

	WSACleanup();
	return 0;
}