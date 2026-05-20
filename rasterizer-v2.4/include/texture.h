#pragma once
#include "math.h"
#include <vector>
#include <fstream>

namespace TEX{
struct Texture{
    int TexHeight;
    int TexWidth;
    std::vector<Vec3> TexData;
};

Texture loadBMP(const char* path);
}
