/*** ���⼭���� �� å�� ��� �������� �������� �����Ͽ� ����ϴ� �ڵ��̴�. ***/

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <iostream>
#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

/*** ��������� �� å�� ��� �������� �������� �����Ͽ� ����ϴ� �ڵ��̴�. ***/
/*** 2�� ������ �������� Common.h�� �����ϴ� ������� �� �ڵ带 ����Ѵ�.  ***/

using namespace std;
#define SERVERPORT 9000
#define BUFSIZE    512
CRITICAL_SECTION cs;
int countid = 0;
HANDLE gamestart = NULL;
int p1x = 0, p1y = 0, p2x = 0, p2y = 0;

struct PlayerSock {
	SOCKET client_sock;
	int x = 0, y = 0;
};

PlayerSock PS[2];


char* recv_filename(SOCKET client_sock, char* buf)
{
	int retval;
	int len;
	retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return NULL;
	}
	else if (retval == 0)
		return NULL;


	retval = recv(client_sock, buf, len, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return NULL;
	}
	else if (retval == 0)
		return NULL;

	buf[len] = '\0'; // ���ϸ��� ���ڿ��� ��ȯ
	return buf;
}
void recv_filesize(SOCKET client_sock, size_t& filesize)
{
	int retval;
	retval = recv(client_sock, (char*)&filesize, sizeof(size_t), MSG_WAITALL);

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return;
	}
	else if (retval == 0)
		return;
}


struct clientinfo
{
	int id;
	SOCKET client;
};

DWORD WINAPI ProcessClient(LPVOID arg)
{
	clientinfo* Info = (clientinfo*)arg;
	int clientId = Info->id;
	SOCKET client_sock = Info->client;
	PS[clientId].client_sock = client_sock;

	WaitForSingleObject(gamestart, INFINITE);

	int retval;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];
	char* filename;
	size_t total_recvdata = 0;
	size_t filesize;
	int len;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	
	if (client_sock == INVALID_SOCKET) {
		MessageBoxA(NULL, "���� ���� ����", "����", MB_OK | MB_ICONERROR);
		WSACleanup();
		return 0;
	}

	while (1)
	{
		int xy[2];

		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		retval = recv(client_sock, (char*)xy, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		PS[(clientId + 1) % 2].x = xy[0];
		PS[(clientId + 1) % 2].y = xy[1];
		


		SOCKET another_client_sock = PS[(clientId + 1) % 2].client_sock;
	
		retval = send(another_client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}
		retval = send(another_client_sock, (const char*)xy, len, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}
	}



	// ���� �ݱ�
	//closesocket(client_sock);
	//delete Info;
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	gamestart = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&cs);

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");


	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	HANDLE hThread;


	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}


		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		clientinfo* info = new clientinfo;
		info->client = client_sock;
		info->id = countid;

		cout << "clientid:" << info->id << endl;


		int retval;
		struct sockaddr_in clientaddr;
		int addrlen;
		char buf[BUFSIZE + 1];
		char* filename;
		size_t total_recvdata = 0;
		size_t filesize;
		int len;

		// Ŭ���̾�Ʈ ���� ���
		addrlen = sizeof(clientaddr);
		getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));


		
		// ������ ����
		if (countid <= 0)
			hThread = CreateThread(NULL, 0, ProcessClient,
				(LPVOID)info, 0, NULL);
		else
			hThread = CreateThread(NULL, 0, ProcessClient,
				(LPVOID)info, 0, NULL);
		
		
		
		countid++;
		if (countid >= 2)
			break;

		//if (hThread == NULL) { closesocket(client_sock); delete info; }
		//else { CloseHandle(hThread); }
	}
	SetEvent(gamestart);
	system("cls");
	while (1)
	{
		int len;

		COORD Pos;

		EnterCriticalSection(&cs);
		system("cls");
		cout << "player1 x:" << PS[0].x << "\tplayer1 y:" << PS[0].y << endl;
		cout << "player2 x:" << PS[1].x << "\tplayer2 y:" << PS[1].y << endl;
		LeaveCriticalSection(&cs);
		Sleep(100);
	}


	// ���� �ݱ�
	closesocket(listen_sock);
	DeleteCriticalSection(&cs);
	// ���� ����
	WSACleanup();
	return 0;
}