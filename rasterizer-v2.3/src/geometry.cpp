#include "geometry.h"
#include <cstdlib>

Cube createCube(Vec3 pos, Vec3 color) {
    Cube cube;
    cube.position = pos;
    cube.color = color;

    Vec3 v[8] = {
        pos + Vec3{0,0,0}, // 0
        pos + Vec3{1,0,0}, // 1
        pos + Vec3{1,1,0}, // 2
        pos + Vec3{0,1,0}, // 3
        pos + Vec3{0,0,1}, // 4
        pos + Vec3{1,0,1}, // 5
        pos + Vec3{1,1,1}, // 6
        pos + Vec3{0,1,1}  // 7
    };

    Vec3 faceColors[6] = {
        {1,0,0}, {0,1,0}, {0,0,1},
        {1,1,0}, {1,0,1}, {0,1,1}
    };

    // FRONT (z = 0)
    cube.faces[0] = {{v[0], faceColors[0]}, {v[1], faceColors[0]}, {v[2], faceColors[0]}};
    cube.faces[1] = {{v[0], faceColors[0]}, {v[2], faceColors[0]}, {v[3], faceColors[0]}};

    // BACK (z = 1)
    cube.faces[2] = {{v[4], faceColors[1]}, {v[5], faceColors[1]}, {v[6], faceColors[1]}};
    cube.faces[3] = {{v[4], faceColors[1]}, {v[6], faceColors[1]}, {v[7], faceColors[1]}};

    // LEFT (x = 0)
    cube.faces[4] = {{v[4], faceColors[2]}, {v[0], faceColors[2]}, {v[3], faceColors[2]}};
    cube.faces[5] = {{v[4], faceColors[2]}, {v[3], faceColors[2]}, {v[7], faceColors[2]}};

    // RIGHT (x = 1)
    cube.faces[6] = {{v[1], faceColors[3]}, {v[5], faceColors[3]}, {v[6], faceColors[3]}};
    cube.faces[7] = {{v[1], faceColors[3]}, {v[6], faceColors[3]}, {v[2], faceColors[3]}};

    // TOP (y = 1)
    cube.faces[8]  = {{v[3], faceColors[4]}, {v[2], faceColors[4]}, {v[6], faceColors[4]}};
    cube.faces[9]  = {{v[3], faceColors[4]}, {v[6], faceColors[4]}, {v[7], faceColors[4]}};

    // BOTTOM (y = 0)
    cube.faces[10] = {{v[4], faceColors[5]}, {v[1], faceColors[5]}, {v[5], faceColors[5]}};
    cube.faces[11] = {{v[4], faceColors[5]}, {v[5], faceColors[5]}, {v[0], faceColors[5]}};

    return cube;
}


// CHUNK //

Chunk createChunk(Vec3 origin, bool fillChunk)
{
    Chunk chunk;

    int cx = (int)origin.x;
    int cy = (int)origin.y;
    int cz = (int)origin.z;

    chunk.origin = Vec3{
        (float)(cx * CHUNK_SIZE),
        (float)(cy * CHUNK_SIZE),
        (float)(cz * CHUNK_SIZE)
    };

    for (int x = 0; x < CHUNK_SIZE; ++x)
    for (int y = 0; y < CHUNK_SIZE; ++y)
    for (int z = 0; z < CHUNK_SIZE; ++z)
    {
        chunk.voxels[x][y][z] = fillChunk;
        chunk.colors[x][y][z] =
        { (float)x/CHUNK_SIZE, (float)y/CHUNK_SIZE, (float)z/CHUNK_SIZE };
    }

    rebuildChunkMesh(chunk);

    return chunk;
}

void rebuildChunkMesh(Chunk& chunk)
{
    chunk.triangles.clear();

    auto empty = [&](int x, int y, int z)
    {
        return x < 0 || y < 0 || z < 0 ||
               x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE ||
               !chunk.voxels[x][y][z];
    };

    for (int x = 0; x < CHUNK_SIZE; ++x)
    for (int y = 0; y < CHUNK_SIZE; ++y)
    for (int z = 0; z < CHUNK_SIZE; ++z)
    {
        if (!chunk.voxels[x][y][z])
            continue;

        Vec3 base = chunk.origin + Vec3{(float)x, (float)y, (float)z};
        Vec3 c = chunk.colors[x][y][z];

        Vec3 v[8] = {
            base + Vec3{0,0,0}, base + Vec3{1,0,0},
            base + Vec3{1,1,0}, base + Vec3{0,1,0},
            base + Vec3{0,0,1}, base + Vec3{1,0,1},
            base + Vec3{1,1,1}, base + Vec3{0,1,1}
        };

        auto addTri = [&](Vertex a, Vertex b, Vertex c)
        {
            chunk.triangles.push_back({a, b, c});
        };

        // FRONT (+Z)
        if (empty(x, y, z + 1)) {
            addTri(
                {v[4], c, {0,0}},
                {v[6], c, {1,1}},
                {v[5], c, {1,0}}
            );
            addTri(
                {v[4], c, {0,0}},
                {v[7], c, {0,1}},
                {v[6], c, {1,1}}
            );
        }

        // BACK (-Z)
        if (empty(x, y, z - 1)) {
            addTri(
                {v[0], c, {0,0}},
                {v[1], c, {1,0}},
                {v[2], c, {1,1}}
            );
            addTri(
                {v[0], c, {0,0}},
                {v[2], c, {1,1}},
                {v[3], c, {0,1}}
            );
        }

        // LEFT (-X)
        if (empty(x - 1, y, z)) {
            addTri(
                {v[0], c, {0,0}},
                {v[3], c, {0,1}},
                {v[7], c, {1,1}}
            );
            addTri(
                {v[0], c, {0,0}},
                {v[7], c, {1,1}},
                {v[4], c, {1,0}}
            );
        }

        // RIGHT (+X)
        if (empty(x + 1, y, z)) {
            addTri(
                {v[1], c, {0,0}},
                {v[5], c, {1,0}},
                {v[6], c, {1,1}}
            );
            addTri(
                {v[1], c, {0,0}},
                {v[6], c, {1,1}},
                {v[2], c, {0,1}}
            );
        }

        // TOP (+Y)
        if (empty(x, y + 1, z)) {
            addTri(
                {v[3], c, {0,0}},
                {v[2], c, {1,0}},
                {v[6], c, {1,1}}
            );
            addTri(
                {v[3], c, {0,0}},
                {v[6], c, {1,1}},
                {v[7], c, {0,1}}
            );
        }

        // BOTTOM (-Y)
        if (empty(x, y - 1, z)) {
            addTri(
                {v[0], c, {0,0}},
                {v[4], c, {0,1}},
                {v[5], c, {1,1}}
            );
            addTri(
                {v[0], c, {0,0}},
                {v[5], c, {1,1}},
                {v[1], c, {1,0}}
            );
        }
    }
}


