#pragma once
#include "math.h"
#include "render.h"
#include "geometry.h"
#include "texture.h"
#include "physics.h"
#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
#include <string>
#include <iostream>

extern bool keys[256];

const float EYE_HEIGHT = 1.6f;
const float PLAYER_HEIGHT = 1.8f;
const float PLAYER_RADIUS = 0.3f;

struct Player {
    Vec3 pos = {2, 5, 2};
    Vec3 vel = {0,0,0};
    bool onGround = false;
};

extern Player player;

bool solidAt(int x, int y, int z);
int findGroundY(int x, int z);
bool collideAt(Vec3 p);
void updatePlayer(float dt);
