#include "Fighter.h"

Fighter::Fighter(int x, int y, LPCWSTR imagePath)
    : x(x), y(y), lives(3), leftBoundary(0), topBoundary(0), rightBoundary(0), bottomBoundary(0)
{
    image = new Image(imagePath);
}

Fighter::~Fighter()
{
    delete image;
}

void Fighter::Draw(HDC hdc)
{
    if (image)
    {
        Gdiplus::Graphics graphics(hdc);
        graphics.DrawImage(image, x, y, image->GetWidth(), image->GetHeight());
    }
}

void Fighter::Move(int dx, int dy)
{
    x += dx;
    y += dy;

    if (x < leftBoundary) x = leftBoundary;
    if (y < topBoundary) y = topBoundary;
    if (x + GetWidth() > rightBoundary) x = rightBoundary - GetWidth();
    if (y + GetHeight() > bottomBoundary) y = bottomBoundary - GetHeight();
}

void Fighter::SetBoundary(int left, int top, int right, int bottom)
{
    leftBoundary = left;
    topBoundary = top;
    rightBoundary = right;
    bottomBoundary = bottom;
}

void Fighter::FireBullet(std::vector<Bullet*>& Player1bullets, int score, int& specialAttackCount, int winWidth)
{
    int bulletX = x + GetWidth() / 2 - 10;
    int bulletY = y - 10;

    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && specialAttackCount > 0)
    {
        for (int i = 0; i < winWidth - 200; i += 50)
        {
            Player1bullets.push_back(new Bullet(i, bulletY + 20, -1, L"resource\\image\\special_bullet.png"));
            Player1bullets.push_back(new Bullet(i, bulletY - 20, -1, L"resource\\image\\special_bullet.png"));
            Player1bullets.push_back(new Bullet(i, bulletY - 60, -1, L"resource\\image\\special_bullet.png"));
        }
        specialAttackCount--;
    }

    if (score >= 1000)
    {
        Player1bullets.push_back(new Bullet(bulletX - 40, bulletY, -1, L"resource\\image\\bullet.png"));
        Player1bullets.push_back(new Bullet(bulletX, bulletY, -1, L"resource\\image\\bullet.png"));
    }
    else
    {
        Player1bullets.push_back(new Bullet(bulletX, bulletY, -1, L"resource\\image\\bullet.png"));
    }
}
void Fighter::FireBullet(int x, int y, std::vector<Bullet*>& Player2bullets, int score, int& specialAttackCount, int winWidth)
{
    int bulletX = x;
    int bulletY = y;

    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && specialAttackCount > 0)
    {
        for (int i = 0; i < winWidth - 200; i += 50)
        {
            Player2bullets.push_back(new Bullet(i, bulletY + 20, -1, L"resource\\image\\special_bullet.png"));
            Player2bullets.push_back(new Bullet(i, bulletY - 20, -1, L"resource\\image\\special_bullet.png"));
            Player2bullets.push_back(new Bullet(i, bulletY - 60, -1, L"resource\\image\\special_bullet.png"));
        }
        specialAttackCount--;
    }

    if (score >= 1000)
    {
        Player2bullets.push_back(new Bullet(bulletX - 40, bulletY, -1, L"resource\\image\\bullet.png"));
        Player2bullets.push_back(new Bullet(bulletX, bulletY, -1, L"resource\\image\\bullet.png"));
    }
    else
    {
        Player2bullets.push_back(new Bullet(bulletX, bulletY, -1, L"resource\\image\\bullet.png"));
    }
}