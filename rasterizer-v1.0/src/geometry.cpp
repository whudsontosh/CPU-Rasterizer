#include "geometry.h"
#include <cstdlib>

Cube createCube(Vec3 pos, Vec3 color) {
    Cube cube;
    cube.position = pos;
    cube.color = color;

    // Define 8 cube vertices
    Vec3 v[8] = {
        pos + Vec3{0,0,0}, // 0: left-bottom-front
        pos + Vec3{1,0,0}, // 1: right-bottom-front
        pos + Vec3{1,1,0}, // 2: right-top-front
        pos + Vec3{0,1,0}, // 3: left-top-front
        pos + Vec3{0,0,1}, // 4: left-bottom-back
        pos + Vec3{1,0,1}, // 5: right-bottom-back
        pos + Vec3{1,1,1}, // 6: right-top-back
        pos + Vec3{0,1,1}  // 7: left-top-back
    };

    // Example colors for each face
    Vec3 faceColors[6] = {
        {1, 0, 0}, // front - red
        {0, 1, 0}, // back - green
        {0, 0, 1}, // left - blue
        {1, 1, 0}, // right - yellow
        {1, 0, 1}, // top - magenta
        {0, 1, 1}  // bottom - cyan
    };

    // Front face
    cube.faces[0] = {{v[0], faceColors[0]}, {v[1], faceColors[0]}, {v[2], faceColors[0]}};
    cube.faces[1] = {{v[0], faceColors[0]}, {v[2], faceColors[0]}, {v[3], faceColors[0]}};

    // Back face
    cube.faces[2] = {{v[5], faceColors[1]}, {v[4], faceColors[1]}, {v[7], faceColors[1]}};
    cube.faces[3] = {{v[5], faceColors[1]}, {v[7], faceColors[1]}, {v[6], faceColors[1]}};

    // Left face
    cube.faces[4] = {{v[4], faceColors[2]}, {v[0], faceColors[2]}, {v[3], faceColors[2]}};
    cube.faces[5] = {{v[4], faceColors[2]}, {v[3], faceColors[2]}, {v[7], faceColors[2]}};

    // Right face
    cube.faces[6] = {{v[1], faceColors[3]}, {v[5], faceColors[3]}, {v[6], faceColors[3]}};
    cube.faces[7] = {{v[1], faceColors[3]}, {v[6], faceColors[3]}, {v[2], faceColors[3]}};

    // Top face
    cube.faces[8]  = {{v[3], faceColors[4]}, {v[2], faceColors[4]}, {v[6], faceColors[4]}};
    cube.faces[9]  = {{v[3], faceColors[4]}, {v[6], faceColors[4]}, {v[7], faceColors[4]}};

    // Bottom face
    cube.faces[10] = {{v[4], faceColors[5]}, {v[5], faceColors[5]}, {v[1], faceColors[5]}};
    cube.faces[11] = {{v[4], faceColors[5]}, {v[1], faceColors[5]}, {v[0], faceColors[5]}};


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

        if (empty(x, y, z+1)) {
            addTri(v[0], v[1], v[2]);
            addTri(v[0], v[2], v[3]);
        }
        if (empty(x, y, z-1)) {
            addTri(v[5], v[4], v[7]);
            addTri(v[5], v[7], v[6]);
        }
        if (empty(x-1, y, z)) {
            addTri(v[4], v[0], v[3]);
            addTri(v[4], v[3], v[7]);
        }
        if (empty(x+1, y, z)) {
            addTri(v[1], v[5], v[6]);
            addTri(v[1], v[6], v[2]);
        }
        if (empty(x, y+1, z)) {
            addTri(v[3], v[2], v[6]);
            addTri(v[3], v[6], v[7]);
        }
        if (empty(x, y-1, z)) {
            addTri(v[4], v[5], v[1]);
            addTri(v[4], v[1], v[0]);
        }
    }

    return chunk;
}


