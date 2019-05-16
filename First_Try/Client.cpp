#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <mstcpip.h>
#include <WINSOCK2.H>  


#include <iostream> 
#include <string>

//定义程序中使用的常量      
#define SERVER_ADDRESS "10.122.193.42" //服务器端IP地址      
#define PORT           51500         //服务器的端口号      
#define MSGSIZE        1024         //收发缓冲区的大小     

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