#pragma once

#include <windows.h>
#include <vector>
#include "Bullet.h"
#include "Enemy.h"
#include "Fighter.h"

// UI ��ư ���� �Լ�
void CreateGameButtons(HWND hWnd, int winWidth, int winHeight, HINSTANCE hInstance);
void ShowInitialUIState(HWND hWnd);
void ShowGameOverMenu(HWND hWnd);
void ShowMenu(HWND hWnd);
void HideMenu(HWND hWnd);

// ���� ���� ���� �Լ�
void HandleSinglePlay(HWND hWnd);
void HandleMultiPlay(HWND hWnd);
void HandleResume(HWND hWnd, bool& paused);
void HandleStart(HWND hWnd, bool& gameStarted, bool& showMenu);
void HandleRestart(
    HWND hWnd,
    std::vector<Bullet*>& Enemybullets,
    std::vector<Bullet*>& Player1bullets,
    std::vector<Bullet*>& Player2bullets,
    std::vector<Enemy*>& enemies,
    Fighter*& playerFighter,
    int& score,
    int& specialAttackCount,
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
