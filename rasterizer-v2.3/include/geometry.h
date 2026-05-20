#pragma once
#include "math.h"
#include <vector>

const int CHUNK_SIZE = 16;

struct Vertex {
    Vec3 position;
    Vec3 color;
    Vec2 uv;
};

// A triangle made of 3 vertices
struct Triangle {
    Vertex v0, v1, v2;
};

struct Cube {
    Vec3 position;
    Vec3 color;
    Triangle faces[12];
};

Cube createCube(Vec3 position, Vec3 color);

struct Chunk {
    Vec3 origin;
    bool voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    Vec3 colors[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    std::vector<Triangle> triangles; // precomputed visible triangles
};

Chunk createChunk(Vec3 origin, bool fillChunk);
void rebuildChunkMesh(Chunk& chunk);
