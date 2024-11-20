#pragma once

void CreateGameButtons(HWND hWnd, int winWidth, int winHeight, HINSTANCE hInstance);
void ShowInitialUIState(HWND hWnd);
void ShowMenu(HWND hWnd);
void HideMenu(HWND hWnd);
void HandleResume(HWND hWnd, bool& paused);
void HandleStart(HWND hWnd, bool& gameStarted, bool& showMenu);
//void HandleRestart(HWND hWnd, GameManager* gameManager);
void HandleRestart(HWND hWnd, std::vector<Bullet*>& bullets, std::vector<Enemy*>& enemies, Fighter*& playerFighter, int& score, int& specialAttackCount, bool& gameStarted, bool& showMenu, bool& paused, bool& gameOver, int winWidth, int winHeight);
void HandleToggleMusic(bool& musicPlaying);
void HandleQuit();
void DisplayScoreAndSpecialAttack(HDC hMemDC, int score, int specialAttackCount);
void DisplayPlayerLives(HDC hMemDC, Fighter* playerFighter, Image* lifeImage);