#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WINSOCK2.H> 
#include <iostream>
#include <windows.h>
#include <process.h> 

#include <iostream>     

#define PORT           51500    //�˿ں�
#define MSGSIZE        1024    


#pragma comment(lib, "ws2_32.lib")      

using namespace std;

HANDLE hMutex;
int my_count=0;

unsigned __stdcall newClient(void* pArguments)
{
	//SOCKET *p = (SOCKET*)pArguments;//ǿ������ת��
	SOCKET sClient = *((SOCKET*)pArguments);


	while (1)
	{
		WaitForSingleObject(hMutex, INFINITE);//

		//��������
		char szMessage[MSGSIZE];
		int ret = recv(sClient, szMessage, MSGSIZE, 0);
		if (ret > 0)
		{
			szMessage[ret] = '\0';
			my_count++;
			cout << "No." << my_count << ":" << szMessage << endl;
		}
		if (ret == 0)/*�ر�*/
		{
			shutdown(sClient, SD_SEND);
			closesocket(sClient);//�����رջ᷵��0
			break;
		}

		//��������
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

	//�����׽���
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		cout << "socket error!" << endl;
		return 0;
	}

	//��IP�Ͷ˿�
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

	if (listen(sListen, 5) == SOCKET_ERROR)
		//����sListen   sListen�������Ŷӵ���������
		/*ÿ����һ���ͻ���connect�ˣ�listen�Ķ����оͼ���һ�����ӣ�
		ÿ����������accept�ˣ��ʹ�listen�Ķ�����ȡ��һ�����ӣ�
		ת��һ��ר�������������ݵ�socket��accept�����ķ���ֵ��*/
	{
		cout << "listen error!\n";
		return 0;
	}

	/*�߳�*/
	hMutex = CreateMutex(NULL, FALSE, NULL);

	/*ѭ��*/
	bool quit = false;
	bool connected = false;
	while (true)
	{
		/*accept()*/
		cout << "waiting for connect...\n";
		SOCKET *sClient = new SOCKET;
		SOCKADDR_IN clientaddr;
		int iaddrSize = sizeof(SOCKADDR_IN);
		*sClient = accept(sListen, (struct sockaddr *) &clientaddr, &iaddrSize);//�����ӻῨ������
		if (*sClient == INVALID_SOCKET)
		{
			cout << "accept error!\n";
		}
		cout << "Accept a connection:" << inet_ntoa(clientaddr.sin_addr) << ":"
			<< ntohs(clientaddr.sin_port) << endl;

		/*�½��߳�*/
		HANDLE hThread;
		hThread=(HANDLE)_beginthreadex(NULL, 0, newClient, sClient, 0, NULL);
		CloseHandle(hThread);
	}

	CloseHandle(hMutex);

	return 0;
}