#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include "Bullet.h"

using namespace Gdiplus;

class Enemy
{
public:
    Enemy(int x, int y, LPCWSTR imagePath);
    virtual ~Enemy();
    virtual void Draw(HDC hdc);
    virtual void Move(); // virtual 키워드 추가
    virtual void Attack(std::vector<Bullet*>& bullets); // virtual 키워드 추가
    bool IsDestroyed() const { return destroyed; }
    void Destroy() { destroyed = true; }
    void TakeDamage() { health--; if (health <= 0) Destroy(); }

    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return image->GetWidth(); }
    int GetHeight() const { return image->GetHeight(); }

protected:
    int x, y;
    int health;
    bool destroyed;
    Image* image;
    int attackTimer; // 총알 발사 주기를 위한 타이머
    int moveTimer;   // 이동 패턴을 위한 타이머
    bool movingDown; // 위아래 움직임 방향 플래그
};
