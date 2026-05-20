#include "world.h"
#include "geometry.h"
#include "math.h"
#include <cstdlib>

// IMPORTANT: actual storage
std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks;


// -------------------------
// coordinate conversion
// -------------------------
ChunkCoord worldToChunk(int x, int y, int z)
{
    return {
        (int)std::floor((float)x / CHUNK_SIZE),
        (int)std::floor((float)y / CHUNK_SIZE),
        (int)std::floor((float)z / CHUNK_SIZE)
    };
}

Vec3i worldToLocal(int x, int y, int z)
{
    ChunkCoord c = worldToChunk(x, y, z);

    return {
        x - c.x * CHUNK_SIZE,
        y - c.y * CHUNK_SIZE,
        z - c.z * CHUNK_SIZE
    };
}


// -------------------------
// world query
// -------------------------
bool solidAtWorld(int x, int y, int z)
{
    ChunkCoord c = worldToChunk(x, y, z);

    auto it = chunks.find(c);
    if (it == chunks.end())
        return false;

    Vec3i l = worldToLocal(x, y, z);

    if (l.x < 0 || l.y < 0 || l.z < 0 ||
        l.x >= CHUNK_SIZE || l.y >= CHUNK_SIZE || l.z >= CHUNK_SIZE)
        return false;

    return it->second.voxels[l.x][l.y][l.z] != 0;
}


// -------------------------
// chunk creation
// -------------------------
Chunk& getOrCreateChunk(const ChunkCoord& coord)
{
    auto it = chunks.find(coord);
    if (it != chunks.end())
        return it->second;

    Chunk chunk = createChunk(
    {
        coord.x * CHUNK_SIZE,
        coord.y * CHUNK_SIZE,
        coord.z * CHUNK_SIZE
    }, true);

    auto [newIt, inserted] = chunks.emplace(coord, std::move(chunk));

    Chunk& c = newIt->second;

    rebuildChunkMesh(c);

    return c;
}


// -------------------------
// streaming system
// -------------------------
void updateChunks(const Vec3& playerPos)
{
    ChunkCoord pc = worldToChunk(
        (int)playerPos.x,
        (int)playerPos.y,
        (int)playerPos.z
    );

    int radius = 4;

    // load
    for (int x = -radius; x <= radius; x++)
    for (int z = -radius; z <= radius; z++)
    {
        ChunkCoord c{pc.x + x, 0, pc.z + z};
        getOrCreateChunk(c);
    }

    // unload
    int maxDist = radius + 2;

    for (auto it = chunks.begin(); it != chunks.end(); )
    {
        int dx = it->first.x - pc.x;
        int dz = it->first.z - pc.z;

        if (std::abs(dx) > maxDist || std::abs(dz) > maxDist)
            it = chunks.erase(it);
        else
            ++it;
    }
}
