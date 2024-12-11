#pragma once

#include <windows.h>
#include <vector>
#include "Fighter.h"
#include "Bullet.h"
#include "Enemy.h"


using namespace std;

class GameManager
{
public:
    int PLAYER_START_X = GetInitPosX();
    int PLAYER_START_Y = 700;

    const int BACKGROUND_SPEED = 4;

    GameManager(int width, int height);
    ~GameManager();

    Fighter*& GetPlayer() { return playerFighter; }
    Fighter*& GetAnotherPlayer() { return anotherplayerFighter; }
    std::vector<Bullet*>& GetEnemyBullets() { return Enemybullets; }
    std::vector<Bullet*>& GetPlayer1Bullets() { return Player1bullets; }
    std::vector<Bullet*>& GetPlayer2Bullets() { return Player2bullets; }
    std::vector<Enemy*>& GetEnemies() { return Enemies; }
    int& GetScore() { return score; }
    int& GetSpecialAttackCount() { return specialAttackCount; }
    bool GetPlayerDead() { return playerdead; }
	bool GetAnotherPlayerDead() { return anotherplayerdead; }

    void SetPlayer(Fighter* newPlayer) { playerFighter = newPlayer; }
	void SetAnotherPlayer(Fighter* newPlayer) { anotherplayerFighter = newPlayer; }
    void SetEnemyBullets(std::vector<Bullet*>& newBullets) { Enemybullets = newBullets; }
    void SetPlayer1Bullets(std::vector<Bullet*>& newBullets) { Player1bullets = newBullets; }
    void SetPlayer2Bullets(std::vector<Bullet*>& newBullets) { Player2bullets = newBullets; }
    void SetEnemies(std::vector<Enemy*>& newEnemies) { Enemies = newEnemies; }

    void SetPlayerDead(bool dead) { playerdead = dead; }
    void SetAnotherPlayerDead(bool dead) { anotherplayerdead = dead; }

	void SetInitPosX(int x) { PLAYER_START_X = x;}
	int GetInitPosX() { return PLAYER_START_X; }

    void SetClientID(int id) { ClientID = id; }
    int GetClientID() { return ClientID; };

	void SetScore(int x) { score = x; }
	void SetSpecialAttackCount(int x) { specialAttackCount = x; }
    void SetLives(int x) { playerFighter->SetLives(x); anotherplayerFighter->SetLives(x); }

    void Initialize();
    void CreatePlayer(HWND hWnd);
    void Update(HWND hWnd, WPARAM wParam);
    void HandleCollisions(HWND hWnd);
    void CreateEnemy(int x, int y);


    void Draw(HDC hMemDC);

private:
    int winWidth;
    int winHeight;
    int backgroundY;
    int score;
    int specialAttackCount;
    int lastThreshold;
    int ClientID;

    bool playerdead = false;
    bool anotherplayerdead = false;
    Fighter* playerFighter;
    Fighter* anotherplayerFighter;
    std::vector<Bullet*> Enemybullets;
    std::vector<Bullet*> Player1bullets;
    std::vector<Bullet*> Player2bullets;
    std::vector<Enemy*> Enemies;
    SOCKET sock;


    void UpdatePlayer(HWND hWnd);
    void UpdateEnemies();
    void UpdateBullets();
    void UpdateSpecialAttackCount();
};

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);


