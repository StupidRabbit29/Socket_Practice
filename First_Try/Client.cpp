#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <mstcpip.h>
#include <WINSOCK2.H>  


#include <iostream> 
#include <string>

//���������ʹ�õĳ���      
#define SERVER_ADDRESS "10.122.193.42" //��������IP��ַ      
#define PORT           51500         //�������Ķ˿ں�      
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
		//		�ͻ���socket   �������˵ĵ�ַ   ��ַ����
	{  //����ʧ�� 
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}

	bool bOptVal = true;
	int bOptLen = sizeof(bool);

	if (setsockopt(sclient, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) == SOCKET_ERROR)
	{
		cout << "setsockopt error!\n";
	}
	else
	{
		cout << "SO_KEEPALIVE is ON\n";
	}

	bool quit = false;
	while (!quit) 
	{
		cout << "Send:";
		char data[100] = { '\0' };
		while(!(cin.getline(data, 100, '\n')&&data[0]!='\0'))
		{		
			cin.clear();
			if(data[0]!='\0')
				cin.ignore(100, '\n');
			memset(data, 0, sizeof(data));
		}
		send(sclient, data, strlen(data), 0);
		//send()������������ָ����socket�����Է�����
		//int send(int s, const void * msg, int len, unsigned int flags)
		//sΪ�ѽ��������ӵ�socket��msgָ���������ݣ�len��Ϊ���ݳ��ȣ�����flagsһ����0
		//�ɹ��򷵻�ʵ�ʴ��ͳ�ȥ���ַ�����ʧ�ܷ���-1������ԭ�����error 

		//�����ڷ���֮�󣬵����޷����̵�֪ϵͳ��æ
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret>0)
		{
			recData[ret] = 0x00;
			cout << recData << endl;
		}

		//TCP_INFO_v0 info;
		if (strcmp(recData, "Server quit!") == 0 || strcmp(recData, "��������æ�����Ժ����ԣ�") == 0)
		{
			quit = true;
			cout << "�ͻ����˳�" << endl;
			Sleep(2000);
			//break;
		}

		/*int iOptVal = 0;
		int iOptLen = sizeof(int);

		if (getsockopt(sclient, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen) == SOCKET_ERROR)
		{
			cout << "getsockopt ERROR\n";
		}
		else
		{
			cout << "SO_KEEPALIVE value: " << iOptVal << endl;
		}*/
	}

	closesocket(sclient);
	WSACleanup();
	return 0;
}