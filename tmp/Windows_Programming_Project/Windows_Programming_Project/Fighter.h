#pragma once
#include <windows.h>
#include <gdiplus.h>

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
    int GetWidth() const { return image->GetWidth(); }
    int GetHeight() const { return image->GetHeight(); }

private:
    int x, y;
    int lives;
    int leftBoundary, topBoundary, rightBoundary, bottomBoundary;
    Image* image;
};
