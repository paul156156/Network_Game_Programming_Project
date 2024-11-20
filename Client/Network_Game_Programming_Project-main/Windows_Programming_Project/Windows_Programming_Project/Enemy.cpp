#include "Enemy.h"
#include <cmath>

Enemy::Enemy(int x, int y, LPCWSTR imagePath)
    : x(x), y(y), health(3), destroyed(false), attackTimer(0), moveTimer(0), movingDown(true)
{
    image = new Image(imagePath);
}

Enemy::~Enemy()
{
    delete image;
}

void Enemy::Draw(HDC hdc)
{
    Graphics graphics(hdc);
    graphics.DrawImage(image, x, y, image->GetWidth(), image->GetHeight());
}

void Enemy::Move()
{
    moveTimer += 50;

    if (moveTimer >= 2000)
    {
        movingDown = !movingDown;
        moveTimer = 0;
    }

    if (movingDown)
    {
        y += 5;
    }
    else
    {
        y -= 5;
    }

    x += 20 * sin(moveTimer * 3.14 / 180);

    if (x < 0) x = 0;
    if (x + image->GetWidth() > 500) x = 500 - image->GetWidth();
    if (y < 0) y = 0;
    if (y + image->GetHeight() > 400) y = 400 - image->GetHeight();
}

void Enemy::Attack(std::vector<Bullet*>& bullets)
{
    attackTimer += 50;
    if (attackTimer >= 2000)
    {
        int bulletX = x + image->GetWidth() / 2 - 10;
        int bulletY = y + image->GetHeight();
        bullets.push_back(new Bullet(bulletX, bulletY, 1, L"resource\\image\\enemy_bullet.png")); // 아래로 발사
        attackTimer = 0;
    }
}
