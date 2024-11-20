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
    Graphics graphics(hdc);
    graphics.DrawImage(image, x, y, image->GetWidth(), image->GetHeight());
}

void Fighter::Move(int dx, int dy)
{
    x += dx;
    y += dy;

    if (x < leftBoundary) x = leftBoundary;
    if (y < topBoundary) y = topBoundary;
    if (x + image->GetWidth() > rightBoundary) x = rightBoundary - image->GetWidth();
    if (y + image->GetHeight() > bottomBoundary) y = bottomBoundary - image->GetHeight();
}

void Fighter::SetBoundary(int left, int top, int right, int bottom)
{
    leftBoundary = left;
    topBoundary = top;
    rightBoundary = right;
    bottomBoundary = bottom;
}
