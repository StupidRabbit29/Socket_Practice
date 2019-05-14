#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WINSOCK2.H>   
#include <iostream> 
#include <string>

//定义程序中使用的常量      
#define SERVER_ADDRESS "127.0.0.1" //服务器端IP地址      
#define PORT           5150         //服务器的端口号      
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

	while (true) 
	{
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
		{  //连接失败 
			printf("connect error !");
			closesocket(sclient);
			return 0;
		}

		string data;
		cin >> data;
		const char * sendData;
		sendData = data.c_str();   //string转const char* 
		send(sclient, sendData, strlen(sendData), 0);
		//send()用来将数据由指定的socket传给对方主机
		//int send(int s, const void * msg, int len, unsigned int flags)
		//s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
		//成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error 

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