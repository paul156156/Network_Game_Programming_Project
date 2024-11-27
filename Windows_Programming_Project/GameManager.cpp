#include <algorithm>
#include "PlayerInfo.h"
#include "GameManager.h"
#include "AdvancedEnemy.h"
#include "UIhandler.h"


GameManager::GameManager(int width, int height)
    : winWidth(width), winHeight(height), backgroundY(0), score(0), specialAttackCount(0), lastThreshold(0), playerFighter(nullptr), anotherplayerFighter(nullptr)
{
}

GameManager::~GameManager()
{
    delete playerFighter;
    delete anotherplayerFighter;
    for (auto bullet : Enemybullets) delete bullet;
    for (auto bullet : Player1bullets) delete bullet;
    for (auto bullet : Player2bullets) delete bullet;
    for (auto enemy : enemies) delete enemy;
}

void GameManager::Initialize()
{
    // 게임 상태 초기화
    backgroundY = 0;
    score = 0;
    specialAttackCount = 0;
    lastThreshold = 0;

    for (auto bullet :  Enemybullets)
    {
        delete bullet;
    }
    Enemybullets.clear();
    for (auto bullet : Player1bullets)
    {
        delete bullet;
    }
    Player1bullets.clear();
    for (auto bullet : Player2bullets)
    {
        delete bullet;
    }
    Player2bullets.clear();

    for (auto enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();
}

void GameManager::CreatePlayer(HWND hWnd)
{
    // 기존 플레이어 객체 삭제
    if (playerFighter)
    {
        delete playerFighter;
        playerFighter = nullptr; // 삭제 후 nullptr로 설정
    }
    if (anotherplayerFighter)
    {
        delete anotherplayerFighter;
        anotherplayerFighter = nullptr; // 삭제 후 nullptr로 설정
    }

    // 새 플레이어 객체 생성
    playerFighter = new Fighter(PLAYER_START_X, PLAYER_START_Y, L"resource\\image\\fighter.png");
    if (!playerFighter)
    {
        MessageBox(hWnd, L"Player fighter initialization failed!", L"Error", MB_OK);
        PostQuitMessage(0);
    }
    anotherplayerFighter = new Fighter(PLAYER_START_X, PLAYER_START_Y, L"resource\\image\\fighter.png");
    if (!anotherplayerFighter)
    {
        MessageBox(hWnd, L"Player fighter initialization failed!", L"Error", MB_OK);
        PostQuitMessage(0);
    }

    playerFighter->SetBoundary(0, 0, winWidth, winHeight);
    anotherplayerFighter->SetBoundary(0, 0, winWidth, winHeight);
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


void GameManager::Update(HWND hWnd, WPARAM wParam)
{
    if (wParam == 1) // 게임 로직 타이머
    {
      

        score += 1;
        backgroundY += BACKGROUND_SPEED;
        if (backgroundY >= 3000) backgroundY = 0;

        UpdatePlayer(hWnd);
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

void GameManager::UpdatePlayer(HWND hWnd)
{
    if (!playerFighter) return;
    if (GetForegroundWindow() != hWnd) return;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) playerFighter->Move(-10, 0);
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) playerFighter->Move(10, 0);
    if (GetAsyncKeyState(VK_UP) & 0x8000) playerFighter->Move(0, -10);
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) playerFighter->Move(0, 10);

    playerFighter->SetBoundary(0, 0, winWidth - 200, winHeight);
}

void GameManager::UpdateEnemies()
{
	for (auto enemy : enemies)
	{
		enemy->Move();
		enemy->Attack(Enemybullets);
	}
}

void GameManager::UpdateBullets()
{
    for (auto bullet : Enemybullets)
    {
        bullet->Update();
    }
    Enemybullets.erase(std::remove_if(Enemybullets.begin(), Enemybullets.end(), [](Bullet* bullet) {
        if (bullet->IsOffScreen() || bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
        }), Enemybullets.end());

    for (auto bullet : Player1bullets)
    {
        bullet->Update();
    }
    Player1bullets.erase(std::remove_if(Player1bullets.begin(), Player1bullets.end(), [](Bullet* bullet) {
        if (bullet->IsOffScreen() || bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
    }), Player1bullets.end());

    for (auto bullet : Player2bullets)
    {
        bullet->Update();
    }
    Player2bullets.erase(std::remove_if(Player2bullets.begin(), Player2bullets.end(), [](Bullet* bullet) {
        if (bullet->IsOffScreen() || bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
    }), Player2bullets.end());
}

void GameManager::HandleCollisions(HWND hWnd)
{
    if (!playerFighter) return;

    for (auto bullet : Enemybullets)
    {
        if (bullet->GetDirection() == 1 &&
            CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
                bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
        {
            playerFighter->TakeDamage();
            bullet->Destroy();
        }

        if (bullet->GetDirection() == 1 &&
            CheckCollision(anotherplayerFighter->GetX(), anotherplayerFighter->GetY(), anotherplayerFighter->GetWidth(), anotherplayerFighter->GetHeight(),
                bullet->GetX(), bullet->GetY(), bullet->GetWidth(), bullet->GetHeight()))
        {
            anotherplayerFighter->TakeDamage();
            bullet->Destroy();
        }
    }

    for (auto enemy : enemies)
    {
        //player1 총알 충돌
        for (auto bullet : Player1bullets)
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
        //player2 총알 충돌
        for (auto bullet : Player2bullets)
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

        // 적과 플레이어1 충돌
        if (CheckCollision(playerFighter->GetX(), playerFighter->GetY(), playerFighter->GetWidth(), playerFighter->GetHeight(),
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight()))
        {
			playerFighter->TakeDamage();
			if (playerFighter->GetLives() <= 0)
			{
				delete playerFighter;
			}
        }  
        // 적과 플레이어2 충돌
        if (CheckCollision(anotherplayerFighter->GetX(), anotherplayerFighter->GetY(), anotherplayerFighter->GetWidth(), anotherplayerFighter->GetHeight(),
            enemy->GetX(), enemy->GetY(), enemy->GetWidth(), enemy->GetHeight()))
        {
            anotherplayerFighter->TakeDamage();
            if (anotherplayerFighter->GetLives() <= 0)
            {
                delete anotherplayerFighter;
            }
        }
    }

    Enemybullets.erase(std::remove_if(Enemybullets.begin(), Enemybullets.end(), [](Bullet* bullet) {
        if (bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
        }), Enemybullets.end());
    Player1bullets.erase(std::remove_if(Player1bullets.begin(), Player1bullets.end(), [](Bullet* bullet) {
        if (bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
    }), Player1bullets.end());
    Player2bullets.erase(std::remove_if(Player2bullets.begin(), Player2bullets.end(), [](Bullet* bullet) {
        if (bullet->IsDestroyed())
        {
            delete bullet;
            return true;
        }
        return false;
    }), Player2bullets.end());




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
    if (anotherplayerFighter)
    {
        anotherplayerFighter->Draw(hMemDC);
    }

    for (auto enemy : enemies)
    {
        enemy->Draw(hMemDC);
    }

    for (auto bullet : Enemybullets)
    {
        bullet->Draw(hMemDC);
    }
    for (auto bullet : Player1bullets)
    {
        bullet->Draw(hMemDC);
    }
    for (auto bullet : Player2bullets)
    {
        bullet->Draw(hMemDC);
    }
}

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return !(x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2);
}

