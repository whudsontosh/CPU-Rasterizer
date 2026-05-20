#include "ray.h"

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

        if (solidAt(x,y,z))
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

void modifyBlock(int x,int y,int z,bool value)
{
    int cx = (int)floor((float)x / CHUNK_SIZE) * CHUNK_SIZE;
    int cy = (int)floor((float)y / CHUNK_SIZE) * CHUNK_SIZE;
    int cz = (int)floor((float)z / CHUNK_SIZE) * CHUNK_SIZE;

    for (Chunk& c : chunks)
    {
        int ox = (int)c.origin.x;
        int oy = (int)c.origin.y;
        int oz = (int)c.origin.z;

        if (ox != cx || oy != cy || oz != cz)
            continue;

        int lx = x - ox;
        int ly = y - oy;
        int lz = z - oz;

        if (lx>=0 && ly>=0 && lz>=0 &&
            lx<CHUNK_SIZE && ly<CHUNK_SIZE && lz<CHUNK_SIZE)
        {
            c.voxels[lx][ly][lz] = value;

            std::cout << "MODIFIED "
                      << x << " " << y << " " << z
                      << " -> " << value << std::endl;
        }

        return;
    }

    std::cout << "NO CHUNK FOUND\n";
}

void handleRay(bool place)
{
    Vec3 forward = {
        -sinf(camYaw)*cosf(camPitch),
        sinf(camPitch),
        -cosf(camYaw)*cosf(camPitch)
    };

    Vec3 hit, normal;

    if (raycast(cameraPos, forward, 6.0f, hit, normal))
    {
        Vec3 target = hit;

        if (place)
        {
            target = hit + normal;
            modifyBlock((int)target.x, (int)target.y, (int)target.z, true);
        }
        else
        {
            modifyBlock((int)hit.x, (int)hit.y, (int)hit.z, false);
        }

        rebuildChunkMesh(chunks[0]);
    }
}
