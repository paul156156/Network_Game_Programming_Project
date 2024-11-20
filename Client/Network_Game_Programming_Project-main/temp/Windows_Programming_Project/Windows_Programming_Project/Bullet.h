#pragma once
#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

class Bullet
{
public:
    Bullet(int x, int y, int direction, LPCWSTR imagePath); // direction 추가
    ~Bullet();
    void Draw(HDC hdc);
    void Update();
    bool IsOffScreen() const;
    int GetDirection() const { return direction; }
    void Destroy() { destroyed = true; }
    bool IsDestroyed() const { return destroyed; }

    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return image->GetWidth(); }
    int GetHeight() const { return image->GetHeight(); }

private:
    int x, y;
    int direction; // 방향: -1은 위, 1은 아래
    bool destroyed;
    Image* image;
};
