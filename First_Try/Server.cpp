#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#include <mstcpip.h>
#include <iostream>   
#include <Windows.h>
#include <process.h>



#define PORT           51500    //端口号
#define MSGSIZE        1024     //最大数据长度
#define MAX_CLIENT     2       //最大客户端口数
//?????
#pragma comment(lib, "ws2_32.lib")   

// tcp keepalive结构体
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

	//创建套接字
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

	//绑定IP和端口
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
		//	socket		绑定给socket的地址（本地地址）  地址的长度
	{
		cout << "bind error!" << endl;
	}

	if (listen(sListen, MAX_CLIENT) == SOCKET_ERROR)
		//监听sListen   sListen最大可以排队的连接数量
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

			const char* echo = "服务器繁忙！请稍后重试！";
			send(MsClient->sClient, echo, strlen(echo), 0);
			//const char* echo1 = "Server quit!";
			//send(MsClient->sClient, echo1, strlen(echo1), 0);

			closesocket(MsClient->sClient);
			delete MsClient;

			cout << "服务器繁忙！" << endl;
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
				cout << "已解除一个HANDLE的占用" << endl;
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

	// 设置心跳参数
	inKeepAlive.onoff = 1;                  // 是否启用
	inKeepAlive.keepalivetime = 3000;       // 在tcp通道空闲1000毫秒后， 开始发送心跳包检测
	inKeepAlive.keepaliveinterval = 500;    // 心跳包的间隔时间是500毫秒

											/*
											补充上面的"设置心跳参数"：
											当没有接收到服务器反馈后，对于不同的Windows版本，客户端的心跳尝试次数是不同的，
											比如， 对于Win XP/2003而言, 最大尝试次数是5次， 其它的Windows版本也各不相同。
											当然啦， 如果是在Linux上， 那么这个最大尝试此时其实是可以在程序中设置的。
											*/


											// 调用接口， 启用心跳机制
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
		//接收数据
		int ret = recv(((MySoc*)pArguments)->sClient, szMessage, MSGSIZE, 0);
		if (ret > 0)
		{
			szMessage[ret] = '\0';
			cout << inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
				<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << "::" << szMessage << endl;
		}

		//发送数据
		const char *sendData;
		if (strcmp(szMessage, "Hello Server") == 0)
		{
			sendData = "Hello Client";
		}
		else if (strcmp(szMessage, "Bye") == 0)
		{
			quit = true;
			cout << "断开连接--";
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

	cout << "删除" << inet_ntoa(((MySoc*)pArguments)->ClientAddr.sin_addr) << ":"
		<< ntohs(((MySoc*)pArguments)->ClientAddr.sin_port) << "服务器端socket" << endl;
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
