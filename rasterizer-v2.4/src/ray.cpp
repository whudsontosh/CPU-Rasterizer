#include "ray.h"
#include "world.h"
#include <cmath>
#include <iostream>

// --------------------
// RAYCAST
// --------------------
bool raycast(Vec3 origin, Vec3 dir, float maxDist, Vec3& hitPos, Vec3& hitNormal)
{
    Vec3 pos = origin;
    Vec3 prev = origin;

    for (float t = 0; t < maxDist; t += 0.05f)
    {
        pos = origin + dir * t;

        int x = (int)floor(pos.x);
        int y = (int)floor(pos.y);
        int z = (int)floor(pos.z);

        if (solidAtWorld(x, y, z))
        {
            hitPos = { (float)x, (float)y, (float)z };

            Vec3 diff = pos - prev;

            if (fabs(diff.x) > fabs(diff.y) && fabs(diff.x) > fabs(diff.z))
                hitNormal = { diff.x > 0 ? -1.0f : 1.0f, 0.0f, 0.0f };
            else if (fabs(diff.y) > fabs(diff.z))
                hitNormal = { 0.0f, diff.y > 0 ? -1.0f : 1.0f, 0.0f };
            else
                hitNormal = { 0.0f, 0.0f, diff.z > 0 ? -1.0f : 1.0f };

            return true;
        }

        prev = pos;
    }

    return false;
}


// --------------------
// MODIFY BLOCK
// --------------------
void modifyBlock(int x, int y, int z, bool value)
{
    ChunkCoord c = worldToChunk(x, y, z);

    auto it = chunks.find(c);
    if (it == chunks.end())
    {
        std::cout << "NO CHUNK FOUND\n";
        return;
    }

    Chunk& chunk = it->second;

    Vec3i l = worldToLocal(x, y, z);

    if (l.x < 0 || l.y < 0 || l.z < 0 ||
        l.x >= CHUNK_SIZE || l.y >= CHUNK_SIZE || l.z >= CHUNK_SIZE)
        return;

    chunk.voxels[l.x][l.y][l.z] = value;

    std::cout << "MODIFIED "
              << x << " " << y << " " << z
              << " -> " << value << std::endl;

    rebuildChunkMesh(chunk);
}


// --------------------
// HANDLE RAY
// --------------------
void handleRay(bool place)
{
    Vec3 forward = {
        -sinf(camYaw) * cosf(camPitch),
        sinf(camPitch),
        -cosf(camYaw) * cosf(camPitch)
    };

    Vec3 hit, normal;

    if (raycast(cameraPos, forward, 6.0f, hit, normal))
    {
        Vec3 target = hit;

        if (place)
        {
            target = hit + normal;
            modifyBlock(
                (int)target.x,
                (int)target.y,
                (int)target.z,
                true
            );
        }
        else
        {
            modifyBlock(
                (int)hit.x,
                (int)hit.y,
                (int)hit.z,
                false
            );
        }
    }
}
