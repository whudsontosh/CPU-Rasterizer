#pragma once
#include "math.h"
#include "render.h"
#include "geometry.h"
#include "texture.h"
#include "physics.h"
#include <windows.h>
#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
#include <string>
#include <iostream>

extern RECT gClientRect;
extern POINT gCenterScreen;

bool raycast(Vec3 origin, Vec3 dir, float maxDist, Vec3& hitPos, Vec3& hitNormal);
void modifyBlock(int x,int y,int z,bool value);
void handleRay(bool place);
