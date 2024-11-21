#pragma once
#include <windows.h>
#include <vector>
#include "Fighter.h"
#include "Bullet.h"
#include "Enemy.h"

class GameManager
{
public:
    const int PLAYER_START_X = 225;
    const int PLAYER_START_Y = 700;
    const int BACKGROUND_SPEED = 4;

    GameManager(int width, int height);
    ~GameManager();



    Fighter*& GetPlayer() { return playerFighter; }
    std::vector<Bullet*>& GetBullets() { return bullets; }
    std::vector<Enemy*>& GetEnemies() { return enemies; }
    int& GetScore() { return score; }
    int& GetSpecialAttackCount() { return specialAttackCount; }

    void SetPlayer(Fighter* newPlayer) { playerFighter = newPlayer; }
	void SetBullets(std::vector<Bullet*>& newBullets) { bullets = newBullets; }
	void SetEnemies(std::vector<Enemy*>& newEnemies) { enemies = newEnemies; }

    void Initialize();
    void CreatePlayer(HWND hWnd);
    void Update(HWND hWnd, WPARAM wParam);
    void HandleCollisions(HWND hWnd);
    void CreateEnemy();

    void Draw(HDC hMemDC);
	void GameOver(HWND hWnd);

private:
    int winWidth;
    int winHeight;
    int backgroundY;
    int score;
    int specialAttackCount;
    int lastThreshold;

    Fighter* playerFighter;
    std::vector<Bullet*> bullets;
    std::vector<Enemy*> enemies;

    void UpdatePlayer();
    void UpdateEnemies();
    void UpdateBullets();
    void UpdateSpecialAttackCount();
};

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
