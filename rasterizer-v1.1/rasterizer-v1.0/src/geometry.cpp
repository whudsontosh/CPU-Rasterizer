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

Chunk createChunk(Vec3 origin)
{
    Chunk chunk;
    chunk.origin = origin;
    chunk.triangles.clear();

    for (int x = 0; x < CHUNK_SIZE; ++x)
    for (int y = 0; y < CHUNK_SIZE; ++y)
    for (int z = 0; z < CHUNK_SIZE; ++z)
    {
        chunk.voxels[x][y][z] = rand() % 2;
        chunk.colors[x][y][z] =
            { (float)x/CHUNK_SIZE, (float)y/CHUNK_SIZE, (float)z/CHUNK_SIZE };
    }

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

        Vec3 base = origin + Vec3{(float)x, (float)y, (float)z};
        Vec3 c = chunk.colors[x][y][z];

        Vec3 v[8] = {
            base + Vec3{0,0,0},
            base + Vec3{1,0,0},
            base + Vec3{1,1,0},
            base + Vec3{0,1,0},
            base + Vec3{0,0,1},
            base + Vec3{1,0,1},
            base + Vec3{1,1,1},
            base + Vec3{0,1,1}
        };

        auto addTri = [&](Vec3 a, Vec3 b, Vec3 d)
        {
            chunk.triangles.push_back({{a,c},{b,c},{d,c}});
        };

        // FRONT (+Z)
        addTri(v[4], v[5], v[6]);
        addTri(v[4], v[6], v[7]);

        // BACK (-Z)
        addTri(v[1], v[0], v[3]);
        addTri(v[1], v[3], v[2]);

        // LEFT (-X)
        addTri(v[0], v[4], v[7]);
        addTri(v[0], v[7], v[3]);

        // RIGHT (+X)
        addTri(v[5], v[1], v[2]);
        addTri(v[5], v[2], v[6]);

        // TOP (+Y)
        addTri(v[3], v[7], v[6]);
        addTri(v[3], v[6], v[2]);

        // BOTTOM (-Y)
        addTri(v[0], v[1], v[5]);
        addTri(v[0], v[5], v[4]);
    }

    return chunk;
}


