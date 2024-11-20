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
#include <iostream>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"RAIDEN";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

const int winWidth = 700;
const int winHeight = 800;

const int PLAYER_START_X = 225;
const int PLAYER_START_Y = 700;
const int BACKGROUND_SPEED = 4;
int BACKGROUND_Y = 0;

ULONG_PTR gdiplusToken;
Fighter* playerFighter = nullptr;
std::vector<Bullet*> bullets;
std::vector<Enemy*> enemies;
Image* lifeImage = nullptr;
int score = 0;
int specialAttackCount = 0;
int lastThreshold = 0;
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

void InitializePlayer(HWND hWnd)
{
    playerFighter = new Fighter(PLAYER_START_X, PLAYER_START_Y, L"resource\\image\\fighter.png");
    if (!playerFighter)
    {
        MessageBox(hWnd, L"Player fighter initialization failed!", L"Error", MB_OK);
        PostQuitMessage(0);
    }
    playerFighter->SetBoundary(0, 0, winWidth, winHeight);
}

void UpdatePlayerFighter()
{
    if (playerFighter == nullptr) return;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        playerFighter->Move(-10, 0);
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        playerFighter->Move(10, 0);
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        playerFighter->Move(0, -10);
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        playerFighter->Move(0, 10);
    }
    playerFighter->SetBoundary(0, 0, winWidth - 200, winHeight);
}

void CreateEnemy()
{
    int x = rand() % (winWidth - 250);
    if (score >= 1000)
    {
        enemies.push_back(new AdvancedEnemy(x, 0, L"resource\\image\\advanced_enemy.png"));
    }
    enemies.push_back(new Enemy(x, 0, L"resource\\image\\enemy.png"));
}

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return !(x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2);
}

void CheckCollisions(HWND hWnd)
{
    if (playerFighter == nullptr) return; // 객체가 nullptr인지 확인

    for (auto bullet : bullets)
    {
        // 플레이어와 적의 총알 충돌
        if (bullet->GetDirection() == 1 && CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
            bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
        {
            playerFighter->TakeDamage();
            bullet->Destroy();
            if (playerFighter->GetLives() <= 0)
            {
                // 게임 오버 처리
                gameStarted = false;
                showMenu = true;
                gameOver = true;
                KillTimer(hWnd, 1);
                KillTimer(hWnd, 2);
                ShowWindow(GetDlgItem(hWnd, 3), SW_SHOW);
                ShowWindow(GetDlgItem(hWnd, 4), SW_SHOW);
                return;
            }
        }
    }

    for (auto enemy : enemies)
    {
        for (auto bullet : bullets)
        {
            if (bullet->GetDirection() == -1 && CheckCollision(enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight(),
                bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
            {
                enemy->TakeDamage();
                bullet->Destroy();
                if (enemy->IsDestroyed())
                {
                    score += 10;
                }
            }
        }

        // 적과 플레이어 충돌
        if (CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight()))
        {
            // 게임 오버 처리
            gameStarted = false;
            showMenu = true;
            gameOver = true;
            KillTimer(hWnd, 1);
            KillTimer(hWnd, 2);
            ShowWindow(GetDlgItem(hWnd, 2), SW_SHOW);
            ShowWindow(GetDlgItem(hWnd, 3), SW_SHOW);
            ShowWindow(GetDlgItem(hWnd, 4), SW_SHOW);
            return;
        }
    }

    // 파괴된 적 삭제
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy* enemy) {
        if (enemy->IsDestroyed())
        {
            delete enemy;
            return true;
        }
        return false;
        }), enemies.end());
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

    // 배경 음악 재생
    PlayBGM(L"resource\\sound\\terran.mp3");

    // 플레이어 생성
    InitializePlayer(hWnd);

    // 적 객체 초기 생성
    CreateEnemy();

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

    delete playerFighter;

    // 총알 객체들 삭제
    for (auto bullet : bullets)
    {
        delete bullet;
    }

    // 적 객체들 삭제
    for (auto enemy : enemies)
    {
        delete enemy;
    }

    // 배경 음악 중지 및 닫기
    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"close bgm", NULL, 0, NULL);

    return (int)Message.wParam;
}

void UpdateEnemies()
{
    for (auto enemy : enemies)
    {
        enemy->Move();
        enemy->Attack(bullets);
    }
}

void UpdateBullets()
{
    for (auto bullet : bullets)
    {
        bullet->Update();
    }

    // 화면을 벗어난 총알 삭제
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet* bullet) {
        if (bullet->IsOffScreen() || bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
        }), bullets.end());
}

void UpdateSpecialAttackCount(int score, int& specialAttackCount, int& lastThreshold)
{
    if (score >= lastThreshold + 1000)
    {
        specialAttackCount++;
        lastThreshold += 1000;
    }
}

void Update(HWND hWnd, WPARAM wParam)
{
    if (wParam == 1) // 게임 로직 타이머
    {
        score += 1;
        BACKGROUND_Y += BACKGROUND_SPEED;
        if (BACKGROUND_Y >= 3000) BACKGROUND_Y = 0;

        UpdatePlayerFighter();
        UpdateEnemies();
        UpdateBullets();
        CheckCollisions(hWnd);
        UpdateSpecialAttackCount(score, specialAttackCount, lastThreshold);

        InvalidateRect(hWnd, NULL, FALSE);
    }
    else if (wParam == 2) // 적 생성 타이머
    {
        CreateEnemy();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static Image* pBackgroundImage = nullptr;
    static LPCWSTR imagePath = L"resource\\image\\bg.png"; // 이미지 파일 경로

    switch (iMessage)
    {
    case WM_CREATE:
        pBackgroundImage = LoadPNG(imagePath);
        lifeImage = LoadPNG(L"resource\\image\\life.png"); // 생명 수 이미지 로드

        CreateGameButtons(hWnd, winWidth, winHeight, g_hInst);
        ShowInitialUIState(hWnd);

        break;

    case WM_TIMER:
        if (!paused && gameStarted)
        {
            Update(hWnd, wParam);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: HandleResume(hWnd, paused); break;
        case 2: HandleStart(hWnd, gameStarted, showMenu); break;
        case 3: HandleRestart(hWnd, bullets, enemies, playerFighter, score, specialAttackCount, gameStarted, showMenu, paused, gameOver, winWidth, winHeight); break;
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

        // 플레이어 전투기 그리기
        if (playerFighter)
        {
            playerFighter->Draw(hMemDC);
        }

        // 적 그리기
        for (auto enemy : enemies)
        {
            enemy->Draw(hMemDC);
        }

        // 총알 그리기
        for (auto bullet : bullets)
        {
            bullet->Draw(hMemDC);
        }

  //      // 점수 표시
  //      SetBkMode(hMemDC, TRANSPARENT);
  //      SetTextColor(hMemDC, RGB(255, 255, 255));
  //      HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
  //      HFONT hOldFont = (HFONT)SelectObject(hMemDC, hFont);
  //      wchar_t scoreText[50];
  //      swprintf_s(scoreText, L"Score: %d", score);
  //      TextOut(hMemDC, 520, 20, scoreText, wcslen(scoreText));
  //      SelectObject(hMemDC, hOldFont);
  //      DeleteObject(hFont);

		//// 특수 공격 가능 횟수 표시
  //      wchar_t specialAttackText[50];
  //      swprintf_s(specialAttackText, L"Special Bullet: %d", specialAttackCount);
  //      TextOut(hMemDC, 520, 80, specialAttackText, wcslen(specialAttackText));

  //      SelectObject(hMemDC, hOldFont);
  //      DeleteObject(hFont);

  //      // 생명 수 표시
  //      if (playerFighter)
  //      {
  //          for (int i = 0; i < playerFighter->GetLives(); ++i)
  //          {
  //              Graphics graphics(hMemDC);
  //              graphics.DrawImage(lifeImage, 520 + i * 40, 100, lifeImage->GetWidth(), lifeImage->GetHeight());
  //          }
  //      }

        // 점수와 특수 공격 가능 횟수 표시
        DisplayScoreAndSpecialAttack(hMemDC, score, specialAttackCount);

        // 생명 수 표시
        DisplayPlayerLives(hMemDC, playerFighter, lifeImage);

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
                playerFighter->FireBullet(bullets, score, specialAttackCount, winWidth);
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
        delete playerFighter;
        delete lifeImage;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, iMessage, wParam, lParam);
    }

    return 0;
}