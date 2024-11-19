#pragma once
#include <windows.h>
#include <vector>
#include "Fighter.h"
#include "Bullet.h"
#include "Enemy.h"

void HandleResume(HWND hWnd, bool& paused);
void HandleStart(HWND hWnd, bool& gameStarted, bool& showMenu);
void HandleRestart(HWND hWnd, std::vector<Bullet*>& bullets, std::vector<Enemy*>& enemies, Fighter*& playerFighter, int& score, int& specialAttackCount, bool& gameStarted, bool& showMenu, bool& paused, bool& gameOver, int winWidth, int winHeight);
void HandleToggleMusic(bool& musicPlaying);
void HandleQuit();