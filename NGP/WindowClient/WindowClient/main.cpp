﻿#pragma once 
#include "sockH.h"
#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <mmsystem.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Fighter.h"
#include "Enemy.h"
#include "AdvancedEnemy.h"
#include "Bullet.h"
#include "UIhandler.h"
#include "GameManager.h"
#include "err.h"


#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")


HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"RAIDEN";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

GameManager* gameManager = nullptr;
#define SERVERPORT 9000
#define BUFSIZE    512
char* SERVERIP = (char*)"127.0.0.1";
char buf[BUFSIZE + 1];

SOCKET sock;
CRITICAL_SECTION cs;
const int winWidth = 700;
const int winHeight = 800;

const int BACKGROUND_SPEED = 4;
int BACKGROUND_Y = 0;

ULONG_PTR gdiplusToken;

Image* lifeImage = nullptr;

bool gameStarted = false;
bool showMenu = false;
bool musicPlaying = true;
bool paused = false;
bool isGameOver = false;

void SendPlayerPos(Fighter& player, SOCKET& sock);
void RecvPlayerPos(Fighter& anotherplayerFighter, SOCKET& sock);
void SendBulletPos(vector<Bullet*>& _bullets, SOCKET& sock);
void RecvBulletPos(GameManager& gameManager, SOCKET& sock);
void RecvEnemy(GameManager& gameManager, SOCKET& sock);
void SendGameStart(SOCKET sock);
void SendGameOver(bool isGameOver, SOCKET sock);
void SendPlayerDead(GameManager& gameManager);
void RecvPlayerDead(GameManager& gameManager);
void InitSocket();

void RecvInitData(SOCKET sock);
bool getInitData = false;

//DWORD WINAPI PlayerThread(LPVOID arg);

Image* LoadPNG(LPCWSTR filePath)
{
    return Image::FromFile(filePath);
}

void PlayBGM(LPCWSTR bgmFilePath)
{
    wchar_t command[256];
    wsprintf(command, L"open \"%s\" type mpegvideo alias bgm", bgmFilePath);
    mciSendString(command, NULL, 0, NULL);
    mciSendString(L"play bgm repeat", NULL, 0, NULL);
}

void CreateDebugConsole()
{
    AllocConsole(); // 콘솔을 할당
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout); // 표준 출력(stdout)을 콘솔에 연결
    freopen_s(&fp, "CONOUT$", "w", stderr); // 표준 에러(stderr)를 콘솔에 연결
    std::cout.clear(); // std::cout 버퍼를 지워줌
    std::cerr.clear(); // std::cerr 버퍼를 지워줌
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    // 디버그 콘솔 생성
    CreateDebugConsole();

    HANDLE hThread;
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hInstance;

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 500, 0, winWidth, winHeight, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // GameManager 객체 생성 및 초기화
    //gameManager = new GameManager(winWidth, winHeight);
    //gameManager->Initialize();

    // 배경 음악 재생
    PlayBGM(L"resource\\sound\\terran.mp3");

    SetTimer(hWnd, 1, 16, NULL);
    //SetTimer(hWnd, 2, 1000, NULL);

    InitSocket();
    InitializeCriticalSection(&cs);
    //hThread = CreateThread(NULL, 0, PlayerThread, NULL, 0, NULL);


    while (GetMessage(&Message, NULL, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    KillTimer(hWnd, 1);
    //KillTimer(hWnd, 2);
    GdiplusShutdown(gdiplusToken);

    delete gameManager;

    // 배경 음악 중지 및 닫기
    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"close bgm", NULL, 0, NULL);

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static Image* pBackgroundImage = nullptr;
    static LPCWSTR imagePath = L"resource\\image\\bg.png"; // 이미지 파일 경로

    switch (iMessage)
    {
    case WM_CREATE:

        if (!gameManager)
        {
            gameManager = new GameManager(winWidth, winHeight);
        }
        gameManager->CreatePlayer(hWnd);

        pBackgroundImage = LoadPNG(imagePath);
        lifeImage = LoadPNG(L"resource\\image\\life.png"); // 생명 수 이미지 로드

        CreateGameButtons(hWnd, winWidth, winHeight, g_hInst);
        ShowInitialUIState(hWnd);

        break;

    case WM_TIMER:
        if (!paused && gameStarted)
        {
            BACKGROUND_Y += BACKGROUND_SPEED;

            if (getInitData == false)
            {
                RecvInitData(sock);
                cout << "recvInitData" << endl;
                getInitData = true;
            }

            gameManager->Update(hWnd, wParam);

            // 플레이어의 생명이 0인지 확인
            if (gameManager->GetPlayer() != nullptr)
            {
                if (gameManager->GetPlayer()->GetLives() <= 0)
                {
                    delete gameManager->GetPlayer();
                    gameManager->SetPlayer(nullptr); // 삭제 후 nullptr로 설정
                    gameManager->SetPlayerDead(true);
                }
            }

            //if (gameManager->GetAnotherPlayer() != nullptr)
            //{
            //    if (gameManager->GetAnotherPlayer()->GetLives() <= 0)
            //    {
            //        delete gameManager->GetAnotherPlayer();
            //        gameManager->SetAnotherPlayer(nullptr); // 삭제 후 nullptr로 설정
            //        gameManager->SetAnotherPlayerDead(true);
            //    }
            //}

            if (gameManager->GetPlayerDead() && gameManager->GetAnotherPlayerDead())
			{
                cout << "둘다 사망" << endl;
				gameStarted = false;
                paused = true;
				isGameOver = true;
				ShowGameOverMenu(hWnd); // 메뉴 표시
			}

            // 서버 좌표 수신 및 적 생성
            //cout << "sendmoveReady" << endl;
            SendPlayerPos(*gameManager->GetPlayer(), sock);
            //cout << "sendmove" << endl;
            RecvPlayerPos(*gameManager->GetAnotherPlayer(), sock);
            //cout << "recv_PlayerMove" << endl;
            SendBulletPos(gameManager->GetPlayer1Bullets(), sock);
            //cout << "SendPlayerBullet" << endl;
            RecvBulletPos(*gameManager, sock);
            //cout << "RecvPlayerBullet" << endl;
            SendPlayerDead(*gameManager);
            //cout << "SendPlayerDead" << endl;
            RecvPlayerDead(*gameManager);
			//cout << "RecvPlayerDead" << endl;
            RecvEnemy(*gameManager, sock);
            //cout << "RecvEnemy" << endl;
        }

        InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신 요청
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: HandleResume(hWnd, paused); break;
        case 2: HandleStart(hWnd, gameStarted, showMenu); 
            SendGameStart(sock);
            break;
        case 3: HandleRestart(hWnd, gameManager->GetEnemyBullets(), gameManager->GetPlayer1Bullets(), gameManager->GetPlayer2Bullets(), gameManager->GetEnemies(), gameManager->GetPlayer(), gameManager->GetAnotherPlayer(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), gameStarted, showMenu, paused, isGameOver, winWidth, winHeight);
            BACKGROUND_Y = 0;
            gameManager->SetPlayerDead(false);
			gameManager->SetAnotherPlayerDead(false);
			//SendGameStart(sock);
            break;
        case 4: HandleToggleMusic(musicPlaying); break;
        case 5: HandleQuit(); break;
        //case 6: HandleSinglePlay(hWnd); break;
        //case 7: HandleMultiPlay(hWnd); break;
        }
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        HDC hMemDC = CreateCompatibleDC(hDC);
        HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, winWidth, winHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

        if (pBackgroundImage)
        {
            Graphics graphics(hMemDC);
            int imgWidth = pBackgroundImage->GetWidth();
            int imgHeight = pBackgroundImage->GetHeight();

            graphics.DrawImage(pBackgroundImage, 0, BACKGROUND_Y - imgHeight, imgWidth, imgHeight);
            graphics.DrawImage(pBackgroundImage, 0, BACKGROUND_Y, imgWidth, imgHeight);
        }

        gameManager->Draw(hMemDC);

        // 점수와 특수 공격 가능 횟수 표시
        DisplayScoreAndSpecialAttack(hMemDC, gameManager->GetScore(), gameManager->GetSpecialAttackCount());

        // 생명 수 표시
        DisplayPlayerLives(hMemDC, gameManager->GetPlayer(), lifeImage);

        BitBlt(hDC, 0, 0, winWidth, winHeight, hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hMemBitmap);
        DeleteDC(hMemDC);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_SPACE:
            if (gameStarted && !paused) // 게임이 시작되고 일시 정지되지 않은 경우에만 총알 발사
            {
                gameManager->GetPlayer()->FireBullet(gameManager->GetPlayer1Bullets(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), winWidth);
            }
            break;
        case VK_ESCAPE:
            if (gameStarted)
            {
                paused = !paused; // 일시 정지
                if (paused)
                {
                    KillTimer(hWnd, 1);
                    //KillTimer(hWnd, 2);

                    // 메뉴 보이기
                    ShowMenu(hWnd);
                }
                else
                {
                    // 게임 다시 시작
                    SetTimer(hWnd, 1, 50, NULL);
                    //SetTimer(hWnd, 2, 1000, NULL);

                    // 메뉴 숨기기
                    HideMenu(hWnd);
                }
            }
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
        break;

    case WM_DESTROY:
        delete pBackgroundImage;
        delete gameManager;
        delete lifeImage;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, iMessage, wParam, lParam);
    }

    return 0;
}

struct CS_MOVE_PLAYER { Fighter& player; };

void SendPlayerPos(Fighter& player, SOCKET& sock)
{
    int xy[2] = { 0, };
    if (&player != nullptr)
    {
        xy[0] = player.GetX();
        xy[1] = player.GetY();
    }
    else
    {
        xy[0] = -100;
        xy[1] = -100;
    }
   
    //cout << xy[0] << " " << xy[1] << endl;
    int retval, len;

    len = sizeof(xy);

    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
        return;
    }
    retval = send(sock, (char*)&len, sizeof(int), 0);

    if (retval == SOCKET_ERROR)
    {
        cout << "err";
        err_display("send()");
        return;
    }

    retval = send(sock, (const char*)xy, len, 0);
    if (retval == SOCKET_ERROR)
    {
        err_display("send()");
        return;
    }
}

void RecvPlayerPos(Fighter& anotherplayerFighter, SOCKET& sock)
{
    int xy[2];
    int retval, len;

    retval = recv(sock, (char*)&len, sizeof(int), MSG_WAITALL);
    if (retval == SOCKET_ERROR)
    {
        err_display("recv()");
        return;
    }
    else if (retval == 0)
        return;

    retval = recv(sock, (char*)xy, len, MSG_WAITALL);
    if (retval == SOCKET_ERROR)
    {
        err_display("recv()");
        return;
    }
    else if (retval == 0)
        return;

    anotherplayerFighter.SetX(xy[0]);
    anotherplayerFighter.SetY(xy[1]);
}

struct BulletData { int x, y; bool destroy, send; };

void SendBulletPos(vector<Bullet*>& _bullets, SOCKET& sock)
{
    vector<BulletData> BD;
    for (auto bullet : _bullets)
    {
        if (bullet->IsSend())
            continue;
        BulletData data = { bullet->GetX(),bullet->GetY(),bullet->IsDestroyed(),bullet->IsSend() };
        BD.push_back(data);
    }


    int retval, len, bulletcnt;
    len = sizeof(BulletData) * BD.size();
    bulletcnt = (int)BD.size();
    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
        return;
    }
    retval = send(sock, (char*)&bulletcnt, sizeof(int), 0);

    if (retval == SOCKET_ERROR)
    {

        err_display("send()");
        return;
    }
    if (bulletcnt == 0)
        return;

    retval = send(sock, (const char*)BD.data(), len, 0);
    if (retval == SOCKET_ERROR)
    {
        err_display("send()");
        return;
    }

    for (auto& bullet : _bullets)
        bullet->Send();
}

void RecvBulletPos(GameManager& gameManager, SOCKET& sock)
{
    int retval, len, bulletcnt = 0;

    retval = recv(sock, (char*)&bulletcnt, sizeof(int), MSG_WAITALL);
    int error_code = WSAGetLastError();
    if (retval == SOCKET_ERROR)
    {
        std::cerr << "Error: recv() failed with error code " << error_code << std::endl;
        err_display("recv()");
        return;
    }
    else if (retval == 0)
        return;

    if (bulletcnt == 0)
        return;

    vector<BulletData> BD(bulletcnt);

    retval = recv(sock, (char*)BD.data(), sizeof(BulletData) * bulletcnt, MSG_WAITALL);
    error_code = WSAGetLastError();
    if (retval == SOCKET_ERROR)
    {
        std::cerr << "Error: recv() failed with error code " << error_code << std::endl;
        err_display("recv()");
        return;
    }
    else if (retval == 0)
        return;

    for (auto bullet : BD)
        gameManager.GetAnotherPlayer()->FireBullet(bullet.x, bullet.y, bulletcnt, gameManager.GetPlayer2Bullets(), gameManager.GetScore(), gameManager.GetSpecialAttackCount(), 700);
}

void RecvEnemy(GameManager& gameManager, SOCKET& sock)
{
    int retval, len = 0;
    int xy[2] = { 0, 0 };

    // 데이터 크기 수신
    retval = recv(sock, (char*)&len, sizeof(len), MSG_WAITALL);
    if (retval == SOCKET_ERROR)
    {
        err_display("recv() - Data length");
        return;
    }
    else if (retval == 0)
    {
        cerr << "Connection closed by server." << endl;
        return;
    }

    // 로그 추가: 데이터 크기 확인
    //cout << "Received data length: " << len << " bytes" << endl;

    // 데이터 크기 검증
    if (len != sizeof(xy))
    {
        cerr << "Unexpected data length: " << len << " (Expected: " << sizeof(xy) << ")" << endl;
        return;
    }

    // 좌표 데이터 수신
    retval = recv(sock, (char*)xy, len, MSG_WAITALL);
    if (retval == SOCKET_ERROR)
    {
        err_display("recv() - Data");
        return;
    }
    else if (retval == 0)
    {
        cerr << "Connection closed by server." << endl; 
        return;
    }

    // 적 생성
    if (xy[0] == 0 && xy[1] == 0)
        return;
    gameManager.CreateEnemy(xy[0], xy[1]);
    cout << "Enemy created at: x=" << xy[0] << ", y=" << xy[1] << endl;
}

void SendGameStart(SOCKET sock) {
    bool isGameStarted = true; // Game Start 플래그
    int retval = send(sock, (char*)&isGameStarted, sizeof(isGameStarted), 0);
    if (retval == SOCKET_ERROR) {
        cerr << "Error sending GameStart: " << WSAGetLastError() << endl;
        return;
    }
    cout << "Game Start message sent to server." << endl;
}

void SendGameOver(bool isGameOver, SOCKET sock) {
    int retval = send(sock, (char*)&isGameOver, sizeof(isGameOver), 0);
    if (retval == SOCKET_ERROR) {
        cerr << "Error sending GameOver: " << WSAGetLastError() << endl;
        return;
    }
    cout << "GameOver message sent to server." << endl;
}

void RecvInitData(SOCKET sock) {
    int clientID;
    int retval;

    // 데이터 길이 수신
    int len;
    retval = recv(sock, (char*)&len, sizeof(len), MSG_WAITALL);
    if (retval == SOCKET_ERROR) {
        err_quit("recv() - 데이터 길이");
    }
    if (retval == 0) {
        cout << "서버 연결이 종료되었습니다." << endl;
        return;
    }

    // 클라이언트 ID 수신
    retval = recv(sock, (char*)&clientID, len, MSG_WAITALL);
    if (retval == SOCKET_ERROR) {
        err_quit("recv() - 클라이언트 ID");
    }
    if (retval == 0) {
        cout << "서버 연결이 종료되었습니다." << endl;
        return;
    }

    cout << "서버로부터 클라이언트 ID를 수신했습니다: " << clientID << endl;

	// 클라이언트 ID 저장
	gameManager->SetPlayerID(clientID);
}

void InitSocket()
{
    int retval;
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        exit(0);

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "소켓 생성 실패", "오류", MB_OK | MB_ICONERROR);
        WSACleanup();
        exit(1);
    }

    int opt_val = TRUE;

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)& opt_val, sizeof(opt_val));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));

    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");


}

void SendPlayerDead(GameManager& gameManager)
{
    int retval;
    bool dead = false;
    if (gameManager.GetPlayerDead())
    {
        cout << "플레이어1 사망" << endl;
        dead = true;
    }
  

    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
        return;
    }
    retval = send(sock, (char*)&dead, sizeof(dead), 0);

    if (retval == SOCKET_ERROR)
    {
        err_display("send()");
        return;
    }

}

void RecvPlayerDead(GameManager& gameManager)
{
    int retval;
    bool dead = false;

    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "유효하지 않은 소켓", "오류", MB_OK | MB_ICONERROR);
        return;
    }
    retval = recv(sock, (char*)&dead, sizeof(dead), 0);

    if (retval == SOCKET_ERROR)
    {
        err_display("recv()");
        return;
    }

    if (dead)
    {
        gameManager.SetAnotherPlayerDead(true);
        cout << "플레이어2 사망" << endl;
    }
    //else
    //    cout << "플레이어2 생존" << endl;
}