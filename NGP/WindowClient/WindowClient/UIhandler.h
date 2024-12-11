#pragma once

#include <windows.h>
#include <vector>
#include "Bullet.h"
#include "Enemy.h"
#include "Fighter.h"
#include "GameManager.h"

// UI 버튼 관리 함수
void CreateGameButtons(HWND hWnd, int winWidth, int winHeight, HINSTANCE hInstance);
void ShowInitialUIState(HWND hWnd);
void ShowGameOverMenu(HWND hWnd);
void ShowMenu(HWND hWnd);
void HideMenu(HWND hWnd);

// 게임 상태 제어 함수
void HandleResume(HWND hWnd, bool& paused);
void HandleStart(HWND hWnd, bool& gameStarted, bool& showMenu);
void HandleRestart(
    HWND hWnd,
	GameManager* gameManager,
    bool& gameStarted,
    bool& showMenu,
    bool& paused,
    bool& gameOver,
    int winWidth,
    int winHeight
);
void HandleToggleMusic(bool& musicPlaying);
void HandleQuit();

// 게임 정보 표시 함수
void DisplayScoreAndSpecialAttack(HDC hMemDC, int score, int specialAttackCount);
void DisplayPlayerLives(HDC hMemDC, Fighter* playerFighter, Gdiplus::Image* lifeImage);
