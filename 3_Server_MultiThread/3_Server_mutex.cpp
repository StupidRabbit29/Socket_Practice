#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WINSOCK2.H> 
#include <iostream>
#include <windows.h>
#include <process.h> 

#include <iostream>     

#define PORT           51500    //端口号
#define MSGSIZE        1024    


#pragma comment(lib, "ws2_32.lib")      

using namespace std;

HANDLE hMutex;
int my_count=0;

unsigned __stdcall newClient(void* pArguments)
{
	//SOCKET *p = (SOCKET*)pArguments;//强制类型转换
	SOCKET sClient = *((SOCKET*)pArguments);


	while (1)
	{
		WaitForSingleObject(hMutex, INFINITE);//

		//接收数据
		char szMessage[MSGSIZE];
		int ret = recv(sClient, szMessage, MSGSIZE, 0);
		if (ret > 0)
		{
			szMessage[ret] = '\0';
			my_count++;
			cout << "No." << my_count << ":" << szMessage << endl;
		}
		if (ret == 0)/*关闭*/
		{
			shutdown(sClient, SD_SEND);
			closesocket(sClient);//正常关闭会返回0
			break;
		}

		//发送数据
		const char *sendData = NULL;
		sendData = "Server: Got it";
		send(sClient, sendData, strlen(sendData), 0);

		ReleaseMutex(hMutex);//
	}

	cout << "disconnected\n";
	delete &sClient;
	_endthreadex(0);
	return 0;
}

int main()
{

	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		cout << "socket error!" << endl;
		return 0;
	}

	//绑定IP和端口
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

	if (listen(sListen, 5) == SOCKET_ERROR)
		//监听sListen   sListen最大可以排队的连接数量
		/*每当有一个客户端connect了，listen的队列中就加入一个连接，
		每当服务器端accept了，就从listen的队列中取出一个连接，
		转成一个专门用来传输数据的socket（accept函数的返回值）*/
	{
		cout << "listen error!\n";
		return 0;
	}

	/*线程*/
	hMutex = CreateMutex(NULL, FALSE, NULL);

	/*循环*/
	bool quit = false;
	bool connected = false;
	while (true)
	{
		/*accept()*/
		cout << "waiting for connect...\n";
		SOCKET *sClient = new SOCKET;
		SOCKADDR_IN clientaddr;
		int iaddrSize = sizeof(SOCKADDR_IN);
		*sClient = accept(sListen, (struct sockaddr *) &clientaddr, &iaddrSize);//无连接会卡在这里
		if (*sClient == INVALID_SOCKET)
		{
			cout << "accept error!\n";
		}
		cout << "Accept a connection:" << inet_ntoa(clientaddr.sin_addr) << ":"
			<< ntohs(clientaddr.sin_port) << endl;

		/*新建线程*/
		HANDLE hThread;
		hThread=(HANDLE)_beginthreadex(NULL, 0, newClient, sClient, 0, NULL);
		CloseHandle(hThread);
	}

	CloseHandle(hMutex);

	return 0;
}