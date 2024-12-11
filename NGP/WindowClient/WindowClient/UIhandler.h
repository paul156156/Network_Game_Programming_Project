#pragma once

#include <windows.h>
#include <vector>
#include "Bullet.h"
#include "Enemy.h"
#include "Fighter.h"
#include "GameManager.h"

// UI ��ư ���� �Լ�
void CreateGameButtons(HWND hWnd, int winWidth, int winHeight, HINSTANCE hInstance);
void ShowInitialUIState(HWND hWnd);
void ShowGameOverMenu(HWND hWnd);
void ShowMenu(HWND hWnd);
void HideMenu(HWND hWnd);

// ���� ���� ���� �Լ�
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

// ���� ���� ǥ�� �Լ�
void DisplayScoreAndSpecialAttack(HDC hMemDC, int score, int specialAttackCount);
void DisplayPlayerLives(HDC hMemDC, Fighter* playerFighter, Gdiplus::Image* lifeImage);
