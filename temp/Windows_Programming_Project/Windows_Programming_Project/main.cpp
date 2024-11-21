#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <mmsystem.h>
#include <vector>
#include <algorithm>
#include "Fighter.h"
#include "Enemy.h"
#include "AdvancedEnemy.h"
#include "Bullet.h"
#include "UIhandler.h"
#include "GameManager.h"
#include <iostream>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"RAIDEN";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

GameManager* gameManager = nullptr;

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

    SetTimer(hWnd, 1, 50, NULL);
    SetTimer(hWnd, 2, 1000, NULL);

    while (GetMessage(&Message, NULL, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    KillTimer(hWnd, 1);
    KillTimer(hWnd, 2);
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
			gameManager->Update(hWnd, wParam);
            InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신 요청
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: HandleResume(hWnd, paused); break;
        case 2: HandleStart(hWnd, gameStarted, showMenu); break;
        case 3: HandleRestart(hWnd, gameManager->GetBullets(), gameManager->GetEnemies(), gameManager->GetPlayer(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), gameStarted, showMenu, paused, gameOver, winWidth, winHeight); 
			BACKGROUND_Y = 0;
            break;
        case 4: HandleToggleMusic(musicPlaying); break;
        case 5: HandleQuit(); break;
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
                gameManager->GetPlayer()->FireBullet(gameManager->GetBullets(), gameManager->GetScore(), gameManager->GetSpecialAttackCount(), winWidth);
            }
            break;
        case VK_ESCAPE:
            if (gameStarted)
            {
                paused = !paused; // 일시 정지
                if (paused)
                {
                    KillTimer(hWnd, 1);
                    KillTimer(hWnd, 2);

                    // 메뉴 보이기
					ShowMenu(hWnd);
                }
                else
                {
                    // 게임 다시 시작
                    SetTimer(hWnd, 1, 50, NULL);
                    SetTimer(hWnd, 2, 1000, NULL);

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