#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#include <mstcpip.h>
#include <iostream>   
#include <Windows.h>
#include <process.h>



#define PORT           51500    //�˿ں�
#define MSGSIZE        1024     //������ݳ���
#define MAX_CLIENT     2       //���ͻ��˿���
//?????
#pragma comment(lib, "ws2_32.lib")   

// tcp keepalive�ṹ��
typedef struct tcp_keepalive TCP_KEEPALIVE;

typedef struct MySoc
{
	SOCKET sClient;
	SOCKADDR_IN ClientAddr;
	int iaddrSize;
	int TrdNum;
}MySoc;

unsigned __stdcall TrdMonitor(void* pArguments);
unsigned __stdcall TrdController(void* pArguments);
unsigned __stdcall MySocketThread(void* pArguments);
bool Full(bool *p, int size);
int GetEmptyTrd(bool *p, int size);

HANDLE monitorTread[MAX_CLIENT];
HANDLE sockThread[MAX_CLIENT];
bool socTrd_USE[MAX_CLIENT] = { false };
bool socTrd_CLOSE[MAX_CLIENT] = { true };

using namespace std;
int main()
{
	//windows socket
	//typedef unsigned short      WORD;
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}

	//�����׽���
	/*SOCKET WSAAPI socket(
		int af,//Address Family
		//AF_INET(IPv4) AF_INET6(IPv6)......
		int type,//socket type
		//SOCK_STREAM SOCK_DGRAM SOCK_RAW......
		int protocol
		//IPPROTO_TCP IPPROTO_UDP......
	);*/
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		cout << "socket error!" << endl;
		return 0;
	}

	//��IP�Ͷ˿�
	//typedef struct sockaddr_in {

	//#if(_WIN32_WINNT < 0x0600)
	//	short   sin_family;
	//#else //(_WIN32_WINNT < 0x0600)
	//	ADDRESS_FAMILY sin_family;
	//#endif //(_WIN32_WINNT < 0x0600)

	//	USHORT sin_port;
	//	IN_ADDR sin_addr;
	//	CHAR sin_zero[8];
	//} SOCKADDR_IN, *PSOCKADDR_IN;
	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT);
	//#define INADDR_ANY              (ULONG)0x00000000
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sListen, (struct sockaddr *) &local, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		//	socket		�󶨸�socket�ĵ�ַ�����ص�ַ��  ��ַ�ĳ���
	{
		cout << "bind error!" << endl;
	}

	if (listen(sListen, MAX_CLIENT) == SOCKET_ERROR)
		//����sListen   sListen�������Ŷӵ���������
	{
		cout << "listen error!\n";
		return 0;
	}
	
	HANDLE Controller= (HANDLE)_beginthreadex(NULL, 0, TrdController, NULL, 0, NULL);
	
	while (true)
	{
		MySoc *MsClient = new MySoc;
		MsClient->iaddrSize = sizeof(SOCKADDR_IN);

		cout << "waiting for connect...\n";
		MsClient->sClient = accept(sListen, (struct sockaddr *) &(MsClient->ClientAddr), &(MsClient->iaddrSize));
		//Server socket      Client socket address    address size
		if (MsClient->sClient == INVALID_SOCKET)
		{
			cout << "accept error!\n";
			//continue;
			return 0;
		}

		cout << "Accept a connection:" << inet_ntoa(MsClient->ClientAddr.sin_addr) << ":"
			<< ntohs(MsClient->ClientAddr.sin_port) << endl;

		if (!Full(socTrd_USE, MAX_CLIENT))
		{
			int TrdNum = GetEmptyTrd(socTrd_USE, MAX_CLIENT);
			socTrd_USE[TrdNum] = true;
			socTrd_CLOSE[TrdNum] = false;
			MsClient->TrdNum = TrdNum;
			sockThread[TrdNum] = (HANDLE)_beginthreadex(NULL, 0, MySocketThread, MsClient, 0, NULL);
			monitorTread[TrdNum] = (HANDLE)_beginthreadex(NULL, 0, TrdMonitor, MsClient, 0, NULL);

		}
		else
		{
			char szMessage[MSGSIZE] = { '\0' };
			recv(MsClient->sClient, szMessage, sizeof(szMessage), 0);

			const char* echo = "��������æ�����Ժ����ԣ�";
			send(MsClient->sClient, echo, strlen(echo), 0);
			//const char* echo1 = "Server quit!";
			//send(MsClient->sClient, echo1, strlen(echo1), 0);

			closesocket(MsClient->sClient);
			delete MsClient;

			cout << "��������æ��" << endl;
			Sleep(3000);
		}
	}

	WaitForSingleObject(Controller, INFINITE);
	CloseHandle(Controller);
	closesocket(sListen);
	WSACleanup();
	return 0;
}

unsigned __stdcall TrdMonitor(void* pArguments)
{
	while (socTrd_CLOSE[((MySoc*)pArguments)->TrdNum]==false)
	{
		char szBuffer[MSGSIZE] = { '\0' };

		int nRet = recv(((MySoc*)pArguments)->sClient, szBuffer, sizeof(szBuffer), MSG_PEEK);

		if (nRet <= 0)
		{
			cout << nRet << endl;
			cout << "HeartBeat detected something" << endl;
			closesocket(((MySoc*)pArguments)->sClient);
			socTrd_CLOSE[((MySoc*)pArguments)->TrdNum] = true;
			//if(socTrd_CLOSE[((MySoc*)pArguments)->TrdNum] == false)
			//	delete (MySoc*)pArguments;

			break;
		}

		Sleep(400);
	}

	_endthreadex(0);
	return 0;
}

unsigned __stdcall TrdController(void* pArguments)
{
	while (true)
	{
		for (int i = 0; i < MAX_CLIENT; i++)
		{
			if (socTrd_USE[i] == true && socTrd_CLOSE[i] == true)
			{
				cout << "�ѽ��һ��HANDLE��ռ��" << endl;
				socTrd_USE[i] = false;
				CloseHandle(sockThread[i]);
				CloseHandle(monitorTread[i]);
			}

		}

		Sleep(3000);
	}

	_endthreadex(0);
	return 0;
}

unsigned __stdcall MySocketThread(void* pArguments)
{
	char szMessage[MSGSIZE] = { '\0' };

	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	
	if (setsockopt(((MySoc*)pArguments)->sClient, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) == SOCKET_ERROR)
	{
		cout << "setsockopt error!\n";
	}
	else
	{
		cout << inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
			<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << "::" << "SO_KEEPALIVE is ON\n";
	}

	TCP_KEEPALIVE inKeepAlive = { 0, 0, 0 };
	unsigned long ulInLen = sizeof(TCP_KEEPALIVE);
	TCP_KEEPALIVE outKeepAlive = { 0, 0, 0 };
	unsigned long ulOutLen = sizeof(TCP_KEEPALIVE);
	unsigned long ulBytesReturn = 0;

	// ������������
	inKeepAlive.onoff = 1;                  // �Ƿ�����
	inKeepAlive.keepalivetime = 3000;       // ��tcpͨ������1000����� ��ʼ�������������
	inKeepAlive.keepaliveinterval = 500;    // �������ļ��ʱ����500����

											/*
											���������"������������"��
											��û�н��յ������������󣬶��ڲ�ͬ��Windows�汾���ͻ��˵��������Դ����ǲ�ͬ�ģ�
											���磬 ����Win XP/2003����, ����Դ�����5�Σ� ������Windows�汾Ҳ������ͬ��
											��Ȼ���� �������Linux�ϣ� ��ô�������Դ�ʱ��ʵ�ǿ����ڳ��������õġ�
											*/


											// ���ýӿڣ� ������������
	WSAIoctl(((MySoc*)pArguments)->sClient, SIO_KEEPALIVE_VALS,
		&inKeepAlive, ulInLen,
		&outKeepAlive, ulOutLen,
		&ulBytesReturn, NULL, NULL);

	/*int KeepAIdle = 100000;
	int KeepAInter = 100;
	int KeepACount = 3;*/
	//setsockopt(((MySoc*)pArguments)->sClient, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&bOptVal, bOptLen);
	//setsockopt(((MySoc*)pArguments)->sClient, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&bOptVal, bOptLen);
	//setsockopt(((MySoc*)pArguments)->sClient, IPPROTO_TCP, TCP_KEEPCNT, (char*)&bOptVal, bOptLen);

	bool quit = false;
	while (!quit)
	{
		memset(szMessage, 0, sizeof(szMessage));
		//��������
		int ret = recv(((MySoc*)pArguments)->sClient, szMessage, MSGSIZE, 0);
		if (ret > 0)
		{
			szMessage[ret] = '\0';
			cout << inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
				<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << "::" << szMessage << endl;
		}

		//��������
		const char *sendData;
		if (strcmp(szMessage, "Hello Server") == 0)
		{
			sendData = "Hello Client";
		}
		else if (strcmp(szMessage, "Bye") == 0)
		{
			quit = true;
			cout << "�Ͽ�����--";
			cout<< inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
				<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << endl;
			sendData = "Server quit!";
		}
		else
		{
			sendData = "Received";
		}
		send(((MySoc*)pArguments)->sClient, sendData, strlen(sendData), 0);

		/*int iOptVal = 0;
		int iOptLen = sizeof(int);

		if (getsockopt(((MySoc*)pArguments)->sClient, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen) == SOCKET_ERROR)
		{
			cout << "getsockopt ERROR\n";
		}
		else
		{
			cout << "SO_KEEPALIVE value: " << iOptVal << endl;
		}*/
	}

	cout << "ɾ��" << inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
		<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << "��������socket" << endl;
	closesocket(((MySoc*)pArguments)->sClient);
	socTrd_CLOSE[((MySoc*)pArguments)->TrdNum] = true;
	delete (MySoc*)pArguments;

	_endthreadex(0);
	return 0;
}

bool Full(bool *p, int size)
{
	for (int i = 0; i < size; i++)
		if (p[i] == false)
			return false;

	return true;
}

int GetEmptyTrd(bool *p, int size)
{
	int i = 0;
	for (; i < size; i++)
		if (p[i] == false)
			break;

	return i;
}
