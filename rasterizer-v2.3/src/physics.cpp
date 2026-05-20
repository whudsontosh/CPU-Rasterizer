#include "physics.h"

bool solidAt(int x, int y, int z)
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

        if (lx < 0 || ly < 0 || lz < 0 ||
            lx >= CHUNK_SIZE || ly >= CHUNK_SIZE || lz >= CHUNK_SIZE)
            return false;

        return c.voxels[lx][ly][lz] != 0;
    }

    return false;
}

int findGroundY(int x, int z)
{
    for (int y = CHUNK_SIZE - 1; y >= 0; y--)
    {
        if (solidAt(x, y, z))
            return y;
    }
    return -1;
}

bool collideAt(Vec3 p)
{
    for (float y = 0; y <= PLAYER_HEIGHT; y += 0.9f)
    {
        for (float x = -PLAYER_RADIUS; x <= PLAYER_RADIUS; x += PLAYER_RADIUS)
        for (float z = -PLAYER_RADIUS; z <= PLAYER_RADIUS; z += PLAYER_RADIUS)
        {
            if (solidAt(
                (int)floor(p.x + x),
                (int)floor(p.y + y),
                (int)floor(p.z + z)))
                return true;
        }
    }
    return false;
}

void updatePlayer(float dt)
{
    const float speed = 6.0f;
    const float gravity = -20.0f;
    const float jumpVel = 8.0f;

    Vec3 forward = { sinf(camYaw), 0, cosf(camYaw) };
    Vec3 right   = { cosf(camYaw), 0,-sinf(camYaw) };

    Vec3 move = {0,0,0};

    if (keys['W']) move = move - forward;
    if (keys['S']) move = move + forward;
    if (keys['A']) move = move - right;
    if (keys['D']) move = move + right;

    if (length(move) > 0)
        move = normalize(move) * speed;

    player.vel.x = move.x;
    player.vel.z = move.z;

    player.vel.y += gravity * dt;

    if (keys[VK_SPACE] && player.onGround)
    {
        player.vel.y = jumpVel;
        player.onGround = false;
    }

    Vec3 next = player.pos;

    // X
    next.x += player.vel.x * dt;
    if (collideAt(next))
    {
        next.x = player.pos.x;
        player.vel.x = 0;
    }

    // Z
    next.z += player.vel.z * dt;
    if (collideAt(next))
    {
        next.z = player.pos.z;
        player.vel.z = 0;
    }

    // Y
    next.y += player.vel.y * dt;

    if (collideAt(next))
    {
        if (player.vel.y < 0)
        {
            player.onGround = true;

            // snap to top of block
            next.y = floor(player.pos.y);
        }

        player.vel.y = 0;
    }
    else
    {
        player.onGround = false;
    }

    player.pos = next;
    cameraPos = player.pos + Vec3{0, EYE_HEIGHT, 0};
}
