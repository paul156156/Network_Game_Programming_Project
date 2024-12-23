/*** 여기서부터 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <iostream>
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <vector>
#include <queue>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

/*** 여기까지가 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/
/*** 2장 이후의 예제들은 Common.h를 포함하는 방식으로 이 코드를 사용한다.  ***/

using namespace std;
#define SERVERPORT 9000
#define BUFSIZE    512
CRITICAL_SECTION cs;
int countid = 0;
HANDLE gamestart = NULL;

struct BulletData { int x, y; bool destroy, send; };
struct PlayerSock {
	SOCKET client_sock;
	vector<BulletData> BulletVector;
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

	buf[len] = '\0'; // 파일명을 문자열로 변환
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
void SendPlayerBullet(PlayerSock* send_PS, PlayerSock* recv_PS)
{
	vector<BulletData> BD;
	
	for (auto bullet : send_PS->BulletVector)
	{
		BulletData data = { bullet.x,bullet.y,bullet.destroy ,bullet.send };
		BD.push_back(data);

	}


	int retval, len, bulletcnt;
	len = sizeof(BulletData) * BD.size();
	bulletcnt = (int)BD.size();
	if (recv_PS->client_sock == INVALID_SOCKET) {
		MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
		return;
	}
	retval = send(recv_PS->client_sock, (char*)&bulletcnt, sizeof(int), 0);
	
	if (bulletcnt == 0)
		return;

	if (retval == SOCKET_ERROR)
	{

		err_display("send()");
		return;
	}
	retval = send(recv_PS->client_sock, (const char*)BD.data(), len, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		return;
	}

	send_PS->BulletVector.clear();
}
void RecvPlayerBullet(PlayerSock* PS)
{
	int retval, len, bulletcnt;
	
	retval = recv(PS->client_sock, (char*)&bulletcnt, sizeof(int), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return;
	}
	else if (retval == 0)
		return;

	if (bulletcnt == 0)
		return;
	vector<BulletData> BD(bulletcnt);
	retval = recv(PS->client_sock, (char*)BD.data(), sizeof(BulletData) * bulletcnt, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return;
	}
	else if (retval == 0)
		return;


	for (auto& bullet : BD)
		PS->BulletVector.push_back(bullet);

	int num = 0;
	EnterCriticalSection(&cs);
	for (auto bullet : PS->BulletVector)
	{
		cout << num << " - " << "x:" << bullet.x << "y:" << bullet.y << endl;
		num++;
	}
	LeaveCriticalSection(&cs);
}
void IsPlayerDead(PlayerSock* PS)
{
	int retval;
	bool dead = false;

	if (PS->client_sock == INVALID_SOCKET) {
		MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
		return;
	}
	retval = recv(PS->client_sock, (char*)&dead, sizeof(dead), 0);

	if (retval == SOCKET_ERROR)
	{
		err_display("recv()");
		return;
	}

	if (dead)
		cout << "플레이어 사망" << endl;
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

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	
	if (client_sock == INVALID_SOCKET) {
		MessageBoxA(NULL, "소켓 생성 실패", "오류", MB_OK | MB_ICONERROR);
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



		RecvPlayerBullet(&PS[clientId]);
		SendPlayerBullet(&PS[clientId], &PS[(clientId + 1) % 2]);
		IsPlayerDead(&PS[clientId]);
		//cout << "본인 클라이언트:" << clientId << "\t" << "보내는 클라이언트:" << (clientId + 1) % 2 << endl;
	}



	// 소켓 닫기
	//closesocket(client_sock);
	//delete Info;
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	gamestart = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&cs);

	// 소켓 생성
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


		// 접속한 클라이언트 정보 출력
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

		// 클라이언트 정보 얻기
		addrlen = sizeof(clientaddr);
		getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));


		
		// 스레드 생성
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

		/*EnterCriticalSection(&cs);
		system("cls");
		cout << "player1 x:" << PS[0].x << "\tplayer1 y:" << PS[0].y << endl;
		cout << "player2 x:" << PS[1].x << "\tplayer2 y:" << PS[1].y << endl;
		LeaveCriticalSection(&cs);
		Sleep(200);*/	
	}


	// 소켓 닫기
	closesocket(listen_sock);
	DeleteCriticalSection(&cs);
	// 윈속 종료
	WSACleanup();
	return 0;
}
