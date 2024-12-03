#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include "Bullet.h"

using namespace Gdiplus;


class Fighter
{
public:
    Fighter(int x, int y, LPCWSTR imagePath);
    ~Fighter();
    void Draw(HDC hdc);
    void Move(int dx, int dy);
    void SetBoundary(int left, int top, int right, int bottom);
    void TakeDamage() { lives--; }
    int GetLives() const { return lives; }

    int GetX() const { return x; }
    int GetY() const { return y; }
    void SetX(int _x) { x = _x; }
    void SetY(int _y) { y = _y; }
    int GetWidth() const { return image ? image->GetWidth() : 0; }
    int GetHeight() const { return image ? image->GetHeight() : 0; }

    void FireBullet(std::vector<Bullet*>& bullets, int score, int& specialAttackCount, int winWidth);
    void FireBullet(int x, int y, std::vector<Bullet*>& Player1bullets, int score, int& specialAttackCount, int winWidth);

private:
    int x, y;
    int lives;
    int leftBoundary, topBoundary, rightBoundary, bottomBoundary;
    Image* image;
};
