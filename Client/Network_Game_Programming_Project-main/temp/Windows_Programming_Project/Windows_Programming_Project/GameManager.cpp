#include "GameManager.h"
#include <algorithm>
#include "AdvancedEnemy.h"

const int PLAYER_START_X = 225;
const int PLAYER_START_Y = 700;
const int BACKGROUND_SPEED = 4;

GameManager::GameManager(int width, int height)
    : winWidth(width), winHeight(height), backgroundY(0), score(0), specialAttackCount(0), lastThreshold(0), playerFighter(nullptr)
{
}

GameManager::~GameManager()
{
    delete playerFighter;
    for (auto bullet : bullets) delete bullet;
    for (auto enemy : enemies) delete enemy;
}

void GameManager::Initialize(HWND hWnd)
{
    playerFighter = new Fighter(PLAYER_START_X, PLAYER_START_Y, L"resource\\image\\fighter.png");
    if (!playerFighter)
    {
        MessageBox(hWnd, L"Player fighter initialization failed!", L"Error", MB_OK);
        PostQuitMessage(0);
    }
    playerFighter->SetBoundary(0, 0, winWidth, winHeight);
}

void GameManager::Update(HWND hWnd, WPARAM wParam)
{
    if (wParam == 1) // 게임 로직 타이머
    {
        score += 1;
        backgroundY += BACKGROUND_SPEED;
        if (backgroundY >= 3000) backgroundY = 0;

        UpdatePlayer();
        UpdateEnemies();
        UpdateBullets();
        HandleCollisions(hWnd);
        UpdateSpecialAttackCount();
    }
    else if (wParam == 2) // 적 생성 타이머
    {
        CreateEnemy();
    }
}

void GameManager::UpdatePlayer()
{
    if (!playerFighter) return;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) playerFighter->Move(-10, 0);
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) playerFighter->Move(10, 0);
    if (GetAsyncKeyState(VK_UP) & 0x8000) playerFighter->Move(0, -10);
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) playerFighter->Move(0, 10);

    playerFighter->SetBoundary(0, 0, winWidth - 200, winHeight);
}

void GameManager::CreateEnemy()
{
    int x = rand() % (winWidth - 250);
    if (score >= 1000)
    {
        enemies.push_back(new AdvancedEnemy(x, 0, L"resource\\image\\advanced_enemy.png"));
    }
    enemies.push_back(new Enemy(x, 0, L"resource\\image\\enemy.png"));
}

void GameManager::UpdateEnemies()
{
	for (auto enemy : enemies)
	{
		enemy->Move();
		enemy->Attack(bullets);
	}
}

void GameManager::UpdateBullets()
{
    for (auto bullet : bullets)
    {
        bullet->Update();
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet* bullet) {
        if (bullet->IsOffScreen() || bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
        }), bullets.end());
}

void GameManager::HandleCollisions(HWND hWnd)
{
    if (!playerFighter) return;

    for (auto bullet : bullets)
    {
        if (bullet->GetDirection() == 1 &&
            CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
                bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
        {
            playerFighter->TakeDamage();
            bullet->Destroy();
            if (playerFighter->GetLives() <= 0)
            {
                // 게임 오버 처리
                PostQuitMessage(0);
            }
        }
    }

    for (auto enemy : enemies)
    {
        for (auto bullet : bullets)
        {
            if (bullet->GetDirection() == -1 &&
                CheckCollision(enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight(),
                    bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
            {
                enemy->TakeDamage();
                bullet->Destroy();
                if (enemy->IsDestroyed()) score += 10;
            }
        }

        if (CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight()))
        {
            // 게임 오버 처리
            PostQuitMessage(0);
        }
    }

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet* bullet) {
        if (bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
        }), bullets.end());

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy* enemy) {
        if (enemy->IsDestroyed())
        {
            delete enemy;
            return true;
        }
        return false;
        }), enemies.end());
}

void GameManager::UpdateSpecialAttackCount()
{
    if (score >= lastThreshold + 1000)
    {
        specialAttackCount++;
        lastThreshold += 1000;
    }
}

void GameManager::Draw(HDC hMemDC)
{
    if (playerFighter)
    {
        playerFighter->Draw(hMemDC);
    }

    for (auto enemy : enemies)
    {
        enemy->Draw(hMemDC);
    }

    for (auto bullet : bullets)
    {
        bullet->Draw(hMemDC);
    }
}

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return !(x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2);
}