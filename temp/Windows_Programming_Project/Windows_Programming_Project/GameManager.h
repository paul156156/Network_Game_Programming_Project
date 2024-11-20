#pragma once
#include <windows.h>
#include <vector>
#include "Fighter.h"
#include "Bullet.h"
#include "Enemy.h"

class GameManager
{
public:
    GameManager(int width, int height);
    ~GameManager();

    void Initialize(HWND hWnd);
    void Update(HWND hWnd, WPARAM wParam);
    void HandleCollisions(HWND hWnd);
    void CreateEnemy();

    Fighter* GetPlayer() const { return playerFighter; }
    const std::vector<Bullet*>& GetBullets() const { return bullets; }
    const std::vector<Enemy*>& GetEnemies() const { return enemies; }

    int GetScore() const { return score; }
    int GetSpecialAttackCount() const { return specialAttackCount; }

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
