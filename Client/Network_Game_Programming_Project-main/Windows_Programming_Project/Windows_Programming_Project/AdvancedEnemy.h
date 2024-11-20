#pragma once
#include "Enemy.h"

class AdvancedEnemy : public Enemy
{
public:
    AdvancedEnemy(int x, int y, LPCWSTR imagePath);
    void Move() override;
    void Attack(std::vector<Bullet*>& bullets) override;
};