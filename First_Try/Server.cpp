#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WINSOCK2.H>   
#include <iostream>     

#define PORT           51500    //端口号
#define MSGSIZE        1024    

//?????
#pragma comment(lib, "ws2_32.lib")      
using namespace std;
int main()
{
	/*照着写*/
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

	if (listen(sListen, 5) == SOCKET_ERROR)
		//监听sListen   sListen最大可以排队的连接数量
	{
		cout << "listen error!\n";
		return 0;
	}
	
	SOCKET sClient;
	SOCKADDR_IN clientaddr;
	char szMessage[MSGSIZE];
	int iaddrSize = sizeof(SOCKADDR_IN);
	

	/*循环*/
	bool quit = false;
	bool connected = false;
	while (true) 
	{
		if (!connected)
		{
			/*accept()*/
			cout << "waiting for connect...\n";
			sClient = accept(sListen, (struct sockaddr *) &clientaddr, &iaddrSize);//无连接会卡在这里
			if (sClient == INVALID_SOCKET)
			{
				cout << "accept error!\n";
			}
			cout << "Accept a connection:" << inet_ntoa(clientaddr.sin_addr) << ":"
				<< ntohs(clientaddr.sin_port) << endl;
			connected = true;
		}

		//接收数据
		int ret = recv(sClient, szMessage, MSGSIZE, 0);
		if (ret > 0)
		{
			szMessage[ret] = '\0';
			cout << szMessage << endl;
		}
		
		//发送数据
		const char *sendData=NULL;
		if (strcmp(szMessage, "Hello Server")==0)
		{
			sendData = "Hello Client";
		}
		//else if (strcmp(szMessage, "quit") == 0)
		//{
		//	//quit = true;
		//	sendData = "Server quit!";
		//	connected=closesocket(sClient);//正常关闭会返回0
		//}
		if (ret == 0)/*关闭*/
		{
			//cout<< "Server quit!";
			//sendData = "Server quit!";
			shutdown(sClient, SD_SEND);
			connected=closesocket(sClient);//正常关闭会返回0
			continue;
		}
		else
		{
			sendData = "Are you OK?";
		}
		send(sClient, sendData, strlen(sendData), 0);
		
	}

	closesocket(sListen);
	WSACleanup();
	return 0;
}
