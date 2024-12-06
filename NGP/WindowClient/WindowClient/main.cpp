#pragma once 
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
const int winWidth = 700;
const int winHeight = 800;

const int PLAYER_START_X = 225;
const int PLAYER_START_Y = 700;
const int BACKGROUND_SPEED = 4;
int BACKGROUND_Y = 0;

ULONG_PTR gdiplusToken;

Image* lifeImage = nullptr;
//int score = 0;
//int specialAttackCount = 0;
//int lastThreshold = 0;
bool gameStarted = false;
bool showMenu = false;
bool musicPlaying = true;
bool paused = false;
bool gameOver = false;

void PlayerMove(Fighter& player, SOCKET& sock);
void recv_PlayerMove(Fighter& anotherplayerFighter, SOCKET& sock);
void SendPlayerBullet(vector<Bullet*>& _bullets, SOCKET& sock);
void RecvPlayerBullet(SOCKET& sock, GameManager& gameManager);
void IsPlayerDead(bool _dead);
void RecvEnemy(GameManager& gameManager, SOCKET& sock);
void SendGameStart(SOCKET sock);
void InitSocket();

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
    AllocConsole(); // �ܼ��� �Ҵ�
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout); // ǥ�� ���(stdout)�� �ֿܼ� ����
    freopen_s(&fp, "CONOUT$", "w", stderr); // ǥ�� ����(stderr)�� �ֿܼ� ����
    std::cout.clear(); // std::cout ���۸� ������
    std::cerr.clear(); // std::cerr ���۸� ������
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    // ����� �ܼ� ����
    //CreateDebugConsole();

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

    // GameManager ��ü ���� �� �ʱ�ȭ
    //gameManager = new GameManager(winWidth, winHeight);
    //gameManager->Initialize();

    // ��� ���� ���
    PlayBGM(L"resource\\sound\\terran.mp3");

    SetTimer(hWnd, 1, 50, NULL);
    SetTimer(hWnd, 2, 1000, NULL);

    InitSocket();

    while (GetMessage(&Message, NULL, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    KillTimer(hWnd, 1);
    KillTimer(hWnd, 2);
    GdiplusShutdown(gdiplusToken);

    delete gameManager;

    // ��� ���� ���� �� �ݱ�
    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"close bgm", NULL, 0, NULL);

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static Image* pBackgroundImage = nullptr;
    static LPCWSTR imagePath = L"resource\\image\\bg.png"; // �̹��� ���� ���

    switch (iMessage)
    {
    case WM_CREATE:
        if (!gameManager)
        {
            gameManager = new GameManager(winWidth, winHeight);
        }
        gameManager->CreatePlayer(hWnd);

        pBackgroundImage = LoadPNG(imagePath);
        lifeImage = LoadPNG(L"resource\\image\\life.png"); // ���� �� �̹��� �ε�

        CreateGameButtons(hWnd, winWidth, winHeight, g_hInst);
        ShowInitialUIState(hWnd);

        break;

    case WM_TIMER:
        if (!paused && gameStarted)
        {
            BACKGROUND_Y += BACKGROUND_SPEED;


            gameManager->Update(hWnd, wParam);

            // �÷��̾��� ������ 0���� Ȯ��
            if (gameManager->GetPlayer()->GetLives() <= 0)
            {
                gameManager->SetPlayerDead(true);
                gameStarted = false;
                paused = true;
                ShowGameOverMenu(hWnd); // �޴� ǥ��
            }

            // ���� ��ǥ ���� �� �� ����
            //RecvEnemy(*gameManager, sock);
            
            PlayerMove(*gameManager->GetPlayer(), sock);
            recv_PlayerMove(*gameManager->GetPlayerAnother(), sock);
            SendPlayerBullet(gameManager->GetPlayer1Bullets(), sock);
            RecvPlayerBullet(sock, *gameManager);
            IsPlayerDead(gameManager->GetPlayerDead());
        }

        InvalidateRect(hWnd, NULL, FALSE); // ȭ�� ���� ��û

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: HandleResume(hWnd, paused); break;
        case 2: HandleStart(hWnd, gameStarted, showMenu); 
            SendGameStart(sock);
            break;
        case 3: HandleRestart(hWnd, gameManager->GetEnemyBullets(), gameManager->GetPlayer1Bullets(), gameManager->GetPlayer2Bullets(), gameManager->GetEnemies(), gameManager->GetPlayer(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), gameStarted, showMenu, paused, gameOver, winWidth, winHeight);
            BACKGROUND_Y = 0;
            gameManager->SetPlayerDead(false);
            break;
        case 4: HandleToggleMusic(musicPlaying); break;
        case 5: HandleQuit(); break;
        case 6: HandleSinglePlay(hWnd); break;
        case 7: HandleMultiPlay(hWnd); break;
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

        // ������ Ư�� ���� ���� Ƚ�� ǥ��
        DisplayScoreAndSpecialAttack(hMemDC, gameManager->GetScore(), gameManager->GetSpecialAttackCount());

        // ���� �� ǥ��
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
            if (gameStarted && !paused) // ������ ���۵ǰ� �Ͻ� �������� ���� ��쿡�� �Ѿ� �߻�
            {
                gameManager->GetPlayer()->FireBullet(gameManager->GetPlayer1Bullets(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), winWidth);
            }
            break;
        case VK_ESCAPE:
            if (gameStarted)
            {
                paused = !paused; // �Ͻ� ����
                if (paused)
                {
                    KillTimer(hWnd, 1);
                    KillTimer(hWnd, 2);

                    // �޴� ���̱�
                    ShowMenu(hWnd);
                }
                else
                {
                    // ���� �ٽ� ����
                    SetTimer(hWnd, 1, 50, NULL);
                    SetTimer(hWnd, 2, 1000, NULL);

                    // �޴� �����
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
void PlayerMove(Fighter& player, SOCKET& sock)
{
    int xy[2];
    xy[0] = player.GetX();
    xy[1] = player.GetY();
    cout << xy[0] << " " << xy[1] << endl;
    int retval, len;

    len = sizeof(xy);

    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "��ȿ���� ���� ����", "����", MB_OK | MB_ICONERROR);
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
void recv_PlayerMove(Fighter& anotherplayerFighter, SOCKET& sock)
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
void SendPlayerBullet(vector<Bullet*>& _bullets, SOCKET& sock)
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
        MessageBoxA(NULL, "��ȿ���� ���� ����", "����", MB_OK | MB_ICONERROR);
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
void RecvPlayerBullet(SOCKET& sock, GameManager& gameManager)
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
        gameManager.GetPlayerAnother()->FireBullet(bullet.x, bullet.y, gameManager.GetPlayer2Bullets(), gameManager.GetScore(), gameManager.GetSpecialAttackCount(), 700);
}

void RecvEnemy(GameManager& gameManager, SOCKET& sock)
{
    int retval, len;
    int xy[2];

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

    gameManager.CreateEnemy(xy[0], xy[1]);
}

void SendGameStart(SOCKET sock)
{
	bool isGameStarted = true;
    send(sock, (char*)&isGameStarted, sizeof(isGameStarted), 0);
}

void InitSocket()
{
    int retval;
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        exit(0);

    // ���� ����
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "���� ���� ����", "����", MB_OK | MB_ICONERROR);
        WSACleanup();
        exit(1);
    }



    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");


}

void IsPlayerDead(bool _dead)
{
    int retval;
    bool dead = _dead;

    if (sock == INVALID_SOCKET) {
        MessageBoxA(NULL, "��ȿ���� ���� ����", "����", MB_OK | MB_ICONERROR);
        return;
    }
    retval = send(sock, (char*)&dead, sizeof(dead), 0);

    if (retval == SOCKET_ERROR)
    {
        err_display("send()");
        return;
    }
}
