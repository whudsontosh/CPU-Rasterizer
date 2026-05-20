#pragma once

#include "geometry.h"
#include "math.h"
#include <unordered_map>
#include <cmath>

struct Vec3i { int x, y, z; };

struct ChunkCoord {
    int x, y, z;

    bool operator==(const ChunkCoord& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const {
        return ((std::hash<int>()(c.x) ^
               (std::hash<int>()(c.y) << 1)) >> 1) ^
               (std::hash<int>()(c.z) << 1);
    }
};

// extern std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks;

// core conversions
ChunkCoord worldToChunk(int x, int y, int z);
Vec3i worldToLocal(int x, int y, int z);

// world queries
bool solidAtWorld(int x, int y, int z);

// chunk system
Chunk& getOrCreateChunk(const ChunkCoord& coord);
void updateChunks(const Vec3& playerPos);
