#include "Bullet.h"

Bullet::Bullet(int x, int y, int direction, LPCWSTR imagePath)
    : x(x), y(y), direction(direction), destroyed(false)
{
    image = new Image(imagePath);
}

Bullet::~Bullet()
{
    delete image;
}

void Bullet::Draw(HDC hdc)
{
    Graphics graphics(hdc);
    graphics.DrawImage(image, x, y, image->GetWidth(), image->GetHeight());
}

void Bullet::Update()
{
    y += direction * 10;
}

bool Bullet::IsOffScreen() const
{
    return y + image->GetHeight() < 0 || y > 800;
}
