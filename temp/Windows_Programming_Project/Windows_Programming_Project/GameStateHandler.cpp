#include <windows.h>
#include <vector>
#include "Fighter.h"
#include "Bullet.h"
#include "Enemy.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

const int PLAYER_START_X = 225;
const int PLAYER_START_Y = 700;

void HandleResume(HWND hWnd, bool& paused) {
    paused = false;
    SetTimer(hWnd, 1, 50, NULL);
    SetTimer(hWnd, 2, 1000, NULL);
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
    SetTimer(hWnd, 2, 1000, NULL); // 1초마다 새로운 적 생성
}

void HandleRestart(HWND hWnd, std::vector<Bullet*>& bullets, std::vector<Enemy*>& enemies, Fighter*& playerFighter, int& score, int& specialAttackCount, bool& gameStarted, bool& showMenu, bool& paused, bool& gameOver, int winWidth, int winHeight) {
    // 게임 상태 초기화
    gameStarted = false;
    showMenu = false;
    score = 0;
    specialAttackCount = 0;
    paused = false;
    gameOver = false;

    // 총알 초기화
    for (auto bullet : bullets) {
        delete bullet;
    }
    bullets.clear();

    // 적 초기화
    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    // 플레이어 초기화
    delete playerFighter;
    playerFighter = new Fighter(PLAYER_START_X, PLAYER_START_Y, L"resource\\image\\fighter.png");
    playerFighter->SetBoundary(0, 0, winWidth, winHeight);

    ShowWindow(GetDlgItem(hWnd, 2), SW_HIDE); // Start
    ShowWindow(GetDlgItem(hWnd, 3), SW_HIDE); // Restart
    ShowWindow(GetDlgItem(hWnd, 4), SW_HIDE); // Toggle Music
    ShowWindow(GetDlgItem(hWnd, 5), SW_HIDE); // Quit
    ShowWindow(GetDlgItem(hWnd, 1), SW_HIDE); // Resume

    SetTimer(hWnd, 1, 50, NULL);
    SetTimer(hWnd, 2, 1000, NULL); // 1초마다 새로운 적 생성
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
