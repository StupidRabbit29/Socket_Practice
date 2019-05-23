#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <mstcpip.h>
#include <WINSOCK2.H>  
#include <mstcpip.h>

#include <iostream> 
#include <string>

//定义程序中使用的常量      
#define SERVER_ADDRESS "10.122.193.42" //服务器端IP地址      
#define PORT           51500         //服务器的端口号      
#define MSGSIZE        1024         //收发缓冲区的大小     

#pragma comment(lib, "ws2_32.lib")      
typedef struct tcp_keepalive TCP_KEEPALIVE;

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

	//远程服务器的地址信息
	SOCKADDR_IN serAddr;
	memset(&serAddr, 0, sizeof(SOCKADDR_IN));
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(PORT);
	serAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		//		客户端socket   服务器端的地址   地址长度
	{  //连接失败 
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

	/*bool bOptVal = true;
	int bOptLen = sizeof(bool);

	if (setsockopt(sclient, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) == SOCKET_ERROR)
	{
		cout << "setsockopt error!\n";
	}
	else
	{
		cout << inet_ntoa(serAddr.sin_addr) << ":"
			<< ntohs(serAddr.sin_port) << "::" << "SO_KEEPALIVE is ON\n";
	}*/

	//心跳检测的参数设定
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
	WSAIoctl(sclient, SIO_KEEPALIVE_VALS,
		&inKeepAlive, ulInLen,
		&outKeepAlive, ulOutLen,
		&ulBytesReturn, NULL, NULL);

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
		//send()用来将数据由指定的socket传给对方主机
		//int send(int s, const void * msg, int len, unsigned int flags)
		//s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
		//成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error 

		//接收在发送之后，导致无法立刻得知系统正忙
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret>0)
		{
			recData[ret] = 0x00;
			cout << recData << endl;
		}

		//TCP_INFO_v0 info;
		if (strcmp(recData, "Server quit!") == 0 || strcmp(recData, "服务器繁忙！请稍后重试！") == 0)
		{
			quit = true;
			cout << "客户端退出" << endl;
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