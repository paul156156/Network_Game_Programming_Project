#include <windows.h>
#include "Fighter.h"
#include "Enemy.h"
#include "GameManager.h"

#pragma comment(lib, "winmm.lib")

void CreateGameButtons(HWND hWnd, int winWidth, int winHeight, HINSTANCE hInstance)
{
    //CreateWindow(
    //    L"BUTTON", L"Single Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
    //    winWidth / 2 - 50, winHeight / 2 - 120, 100, 40, hWnd, (HMENU)6, hInstance, NULL
    //);

    //CreateWindow(
    //    L"BUTTON", L"Multi Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
    //    winWidth / 2 - 50, winHeight / 2 - 70, 100, 40, hWnd, (HMENU)7, hInstance, NULL
    //);

    CreateWindow(
        L"BUTTON", L"Resume", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        winWidth / 2 - 50, winHeight / 2 - 100, 100, 40, hWnd, (HMENU)1, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"Start", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        winWidth / 2 - 50, winHeight / 2 - 50, 100, 40, hWnd, (HMENU)2, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"Restart", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        winWidth / 2 - 50, winHeight / 2 - 50, 100, 40, hWnd, (HMENU)3, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"Toggle Music", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        winWidth / 2 - 50, winHeight / 2, 100, 40, hWnd, (HMENU)4, hInstance, NULL
    );

    CreateWindow(
        L"BUTTON", L"Quit", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        winWidth / 2 - 50, winHeight / 2 + 50, 100, 40, hWnd, (HMENU)5, hInstance, NULL
    );
}

void ShowMenu(HWND hWnd)
{
    ShowWindow(GetDlgItem(hWnd, 1), SW_SHOW); // Resume
    ShowWindow(GetDlgItem(hWnd, 3), SW_SHOW); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_SHOW); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_SHOW); // Quit
}

void HideMenu(HWND hWnd)
{
    ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE); // Resume
    ShowWindow(GetDlgItem(hWnd, 3), SW_HIDE); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_HIDE); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_HIDE); // Quit
}

void ShowInitialUIState(HWND hWnd)
{
    //ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, 2), SW_SHOW);
    //ShowWindow(GetDlgItem(hWnd, 3), SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, 4), SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, 5), SW_SHOW);

}

void ShowGameOverMenu(HWND hWnd)
{
    ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE); // Resume
    ShowWindow(GetDlgItem(hWnd, 2), SW_HIDE); // Start
    ShowWindow(GetDlgItem(hWnd, 3), SW_SHOW); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_SHOW); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_SHOW); // Quit
}

void HandleResume(HWND hWnd, bool& paused) {
    paused = false;
    SetTimer(hWnd, 1, 50, NULL);
    ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE); // Resume
    ShowWindow(GetDlgItem(hWnd, 3), SW_HIDE); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_HIDE); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_HIDE); // Quit
}

void HandleStart(HWND hWnd, bool& gameStarted, bool& showMenu) {
    gameStarted = true;
    showMenu = false;
    ShowWindow(GetDlgItem(hWnd, 2), SW_HIDE); // Start
    ShowWindow(GetDlgItem(hWnd, 4), SW_HIDE); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_HIDE); // Quit
    SetTimer(hWnd, 1, 50, NULL);
}

void HandleRestart(HWND hWnd, GameManager* gameManager, bool& gameStarted, bool& showMenu, bool& paused, bool& gameOver, int winWidth, int winHeight)
{
    // 게임 상태 초기화
    gameManager->SetScore(0);
    gameManager->SetSpecialAttackCount(0);

    gameStarted = false;
    showMenu = false;
    paused = false;
    gameOver = false;

	gameManager->Initialize();
    gameManager->CreatePlayer(hWnd);

    ShowWindow(GetDlgItem(hWnd, 2), SW_HIDE); // Start
    ShowWindow(GetDlgItem(hWnd, 3), SW_HIDE); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_HIDE); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_HIDE); // Quit
    ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE); // Resume

    SetTimer(hWnd, 1, 50, NULL);
    gameStarted = true;
}

void HandleToggleMusic(bool& musicPlaying) {
    if (musicPlaying) {
        mciSendString(L"stop bgm", NULL, 0, NULL);
    }
    else {
        mciSendString(L"open \"resource\\sound\\terran.mp3\" type mpegvideo alias bgm", NULL, 0, NULL);
        mciSendString(L"play bgm repeat", NULL, 0, NULL);
    }
    musicPlaying = !musicPlaying;
}

void HandleQuit() {
    PostQuitMessage(0);
}

void DisplayScoreAndSpecialAttack(HDC hMemDC, int score, int specialAttackCount)
{
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(255, 255, 255));
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hMemDC, hFont);

    // 점수 표시
    wchar_t scoreText[50];
    swprintf_s(scoreText, L"Score: %d", score);
    TextOut(hMemDC, 520, 20, scoreText, wcslen(scoreText));

    // 특수 공격 가능 횟수 표시
    wchar_t specialAttackText[50];
    swprintf_s(specialAttackText, L"Special Bullet: %d", specialAttackCount);
    TextOut(hMemDC, 520, 80, specialAttackText, wcslen(specialAttackText));

    SelectObject(hMemDC, hOldFont);
    DeleteObject(hFont);
}

void DisplayPlayerLives(HDC hMemDC, Fighter* playerFighter, Image* lifeImage)
{
    if (playerFighter)
    {
        for (int i = 0; i < playerFighter->GetLives(); ++i)
        {
            Graphics graphics(hMemDC);
            graphics.DrawImage(lifeImage, 520 + i * 40, 100, lifeImage->GetWidth(), lifeImage->GetHeight());
        }
    }
}