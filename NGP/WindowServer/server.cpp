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
#include <chrono>

#include <random>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distrib(1, 400); 
bool isRunning = true;
bool isGameRunning = false; // 모든 클라이언트가 준비되었는지 여부

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
HANDLE PlayerInfoSend = NULL;
HANDLE semaphore;
bool clientReady[2] = { false, false };


struct BulletData { int x, y; bool destroy, send; };
struct PlayerSock {
	SOCKET client_sock;
	vector<BulletData> BulletVector;
	int x = 0, y = 0;
	bool isGameStarted = false;
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

void SendClientID(PlayerSock* PS, int clientId) {
	int len = sizeof(clientId); // 전송할 데이터의 크기 계산

	// 데이터 길이 전송
	int retval = send(PS->client_sock, (char*)&len, sizeof(len), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() 데이터 길이");
		return;
	}

	// 클라이언트 ID 전송
	retval = send(PS->client_sock, (char*)&clientId, len, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() 클라이언트 ID");
		return;
	}

	// 전송 성공 로그
	cout << "클라이언트 ID 전송 완료: 클라이언트 ID=" << clientId << endl;
}

void RecvGameStart(PlayerSock* PS, int clientId) {
	bool isGameStarted = false;
	int retval = recv(PS->client_sock, (char*)&isGameStarted, sizeof(isGameStarted), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv() GameStart");
		return;
	}

	PS->isGameStarted = isGameStarted;
	clientReady[clientId] = isGameStarted; // 클라이언트 준비 상태 저장
	cout << "Client " << clientId << " is ready: " << isGameStarted << endl;

	// 모든 클라이언트가 준비되었는지 확인
	if (clientReady[0] && clientReady[1]) {
		cout << "All clients are ready. Starting the game!" << endl;
		SetEvent(gamestart); // 이벤트 신호 설정
		isGameRunning = true; // 게임 시작
	}
}

struct clientinfo
{
	int id;
	SOCKET client;
};

DWORD WINAPI EnemySenderThread(LPVOID arg)
{
	//PlayerSock* PS = (PlayerSock*)arg; // 클라이언트 소켓 배열
	const int clientCount = 2; // 최대 클라이언트 수
	auto lastSentTime = chrono::steady_clock::now();

	while (isRunning)
	{
		
		for (int i = 0; i < clientCount; i++)
		{
			WaitForSingleObject(semaphore, INFINITE); // 세마포어 대기
		}
		//Sleep(1); // 1초마다 적 좌표 생성 및 전송

		if (!isGameRunning) continue; // 게임이 진행 중이 아니면 건너뜀

		auto currentTime = chrono::steady_clock::now();
		auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - lastSentTime);
		// 적 좌표 생성
		int xy[2];

		if (elapsedTime.count() >= 1000) // 1초 이상 경과 시
		{
			xy[0] = distrib(gen); // 무작위 x 좌표 생성
			xy[1] = distrib(gen); // 무작위 y 좌표 생성
			lastSentTime = currentTime; // 마지막 전송 시간 갱신
		}
		else
		{
			xy[0] = 0; // 무작위 x 좌표 생성
			xy[1] = 0; // 무작위 y 좌표 생성
		}
		int len = sizeof(xy);

		//cout << "Sending data length: " << len << " bytes" << endl;
		//cout << "Generated enemy position: x=" << xy[0] << ", y=" << xy[1] << endl;

		// 클라이언트들에게 좌표 전송
		for (int i = 0; i < clientCount; i++)
		{
			if (PS[i].client_sock != INVALID_SOCKET) // 유효한 소켓인지 확인
			{
				// 데이터 길이 전송
				int retval = send(PS[i].client_sock, (char*)&len, sizeof(len), 0);
				if (retval == SOCKET_ERROR)
				{
					cerr << "Error sending data length to client " << i
						<< ": " << WSAGetLastError() << endl;
					continue; // 오류 발생 시 해당 클라이언트 건너뜀
				}

				// 좌표 데이터 전송
				retval = send(PS[i].client_sock, (char*)xy, len, 0);
				if (retval == SOCKET_ERROR)
				{
					cerr << "Error sending enemy position to client " << i
						<< ": " << WSAGetLastError() << endl;
					continue; // 오류 발생 시 해당 클라이언트 건너뜀
				}

				//cout << "Sent enemy position to client " << i
				//	<< ": x=" << xy[0] << ", y=" << xy[1] << endl;
			}
			
		}
		cout << "enemy send 완료" << endl;
	}

	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	clientinfo* Info = (clientinfo*)arg;
	int clientId = Info->id;
	SOCKET client_sock = Info->client;
	PS[clientId].client_sock = client_sock;


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

	// 클라이언트 ID 전송
	SendClientID(&PS[clientId], clientId);

	// Game Start 메시지 수신 대기
	RecvGameStart(&PS[clientId], clientId);

	//WaitForSingleObject(gamestart, INFINITE); // 게임 시작 대기
	

	while (1)
	{
		int xy[2];
		//Sleep(1);
		//cout << "start" << endl;
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
		//cout << "moverecv" << endl;
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

		

		//cout << "movesend" << endl;



		RecvPlayerBullet(&PS[clientId]);
		//cout << "bulrecv" << endl;
		SendPlayerBullet(&PS[clientId], &PS[(clientId + 1) % 2]);
		//cout << "bulsend" << endl;
		IsPlayerDead(&PS[clientId]);

		cout << "playerinfo send 완료" << endl;

		
		ReleaseSemaphore(semaphore, 1, NULL);

		//cout << "dead" << endl;
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
	cout << "한글출력 테스트" << endl;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	gamestart = CreateEvent(NULL, TRUE, FALSE, NULL);
	semaphore = CreateSemaphore(NULL, 0, 2, NULL);
	InitializeCriticalSection(&cs);

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	int opt_val = TRUE;
	setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));


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

	HANDLE hEnemyThread = CreateThread(NULL, 0, EnemySenderThread, (LPVOID)PS, 0, NULL);
	if (hEnemyThread == NULL) {
		cerr << "Failed to create enemy sender thread." << endl;
		return 1;
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


	isRunning = false; // 서버 종료 플래그 설정
	WaitForSingleObject(hEnemyThread, INFINITE); // 스레드 종료 대기
	CloseHandle(hEnemyThread);


	// 소켓 닫기
	closesocket(listen_sock);
	DeleteCriticalSection(&cs);
	// 윈속 종료
	WSACleanup();
	return 0;
}
