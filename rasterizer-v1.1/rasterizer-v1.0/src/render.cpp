#include "math.h"
#include "geometry.h"
#include "render.h"
#include <windows.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>

const int WIDTH  = 800;
const int HEIGHT = 600;

Vec3 cameraPos = {0, 0, 0};
float camYaw   = 0.0f;
float camPitch = 0.0f;

POINT lastMouse;
bool mouseCaptured = false;

const float MOUSE_SENS = 0.002f;

std::vector<Chunk> chunks;

Vec3 framebuffer[WIDTH * HEIGHT];
float zbuffer[WIDTH * HEIGHT];


// ----------------------------
// Proper frustum check (AABB vs clip volume)
// ----------------------------
bool chunkInFrustum(const Chunk& chunk, const Mat4& vp)
{
    Vec3 min = chunk.origin;
    Vec3 max = chunk.origin + Vec3{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE};

    Vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, max.y, min.z},
        {min.x, max.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z}
    };

    int inside = 0;

    for (int i = 0; i < 8; i++)
    {
        Vec4 c = vp * Vec4{corners[i].x, corners[i].y, corners[i].z, 1.0f};

        // clip-space rejection
        if (c.x < -c.w || c.x > c.w ||
            c.y < -c.w || c.y > c.w ||
            c.z < -c.w || c.z > c.w)
        {
            continue;
        }

        inside++;
    }

    return inside > 0;
}


// ----------------------------
// Render chunk
// ----------------------------
void renderChunk(const Chunk& chunk, const Mat4& view, const Mat4& proj)
{
    Mat4 vp = proj * view;

    if (!chunkInFrustum(chunk, vp))
        return;

    Vec3 chunkCenter = chunk.origin + Vec3{CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f};
    Vec3 toCam = chunkCenter - cameraPos;

    float dist2 =
        toCam.x * toCam.x +
        toCam.y * toCam.y +
        toCam.z * toCam.z;

    if (dist2 > 200.0f * 200.0f)
        return;

    for (const Triangle& tri : chunk.triangles)
    {
        // --- WORLD - VIEW (for culling) ---
        Vec4 v0v = view * Vec4{tri.v0.position.x, tri.v0.position.y, tri.v0.position.z, 1};
        Vec4 v1v = view * Vec4{tri.v1.position.x, tri.v1.position.y, tri.v1.position.z, 1};
        Vec4 v2v = view * Vec4{tri.v2.position.x, tri.v2.position.y, tri.v2.position.z, 1};

        Vec3 v0_view = { v0v.x, v0v.y, v0v.z };
        Vec3 v1_view = { v1v.x, v1v.y, v1v.z };
        Vec3 v2_view = { v2v.x, v2v.y, v2v.z };

        Vec3 e1v = v1_view - v0_view;
        Vec3 e2v = v2_view - v0_view;

        Vec3 normal = cross(e1v, e2v);

        // use triangle center instead of a vertex
        Vec3 center = (v0_view + v1_view + v2_view) * (1.0f / 3.0f);

        // camera is at origin in view space
        Vec3 viewDir = -center;

        if (dot(normal, viewDir) <= 0.0f)
            continue;

        // --- VIEW - CLIP ---
        Vec4 v0 = proj * v0v;
        Vec4 v1 = proj * v1v;
        Vec4 v2 = proj * v2v;

        // robust per-plane rejection (fixes over-culling)
        auto outside = [](float a, float b, float c, float w)
        {
            return (a < -w && b < -w && c < -w) ||
                   (a >  w && b >  w && c >  w);
        };

        if (outside(v0.x, v1.x, v2.x, v0.w) ||
            outside(v0.y, v1.y, v2.y, v0.w) ||
            outside(v0.z, v1.z, v2.z, v0.w))
            continue;

        // --- Perspective divide (CRITICAL FIX) ---
        float iw0 = 1.0f / v0.w;
        float iw1 = 1.0f / v1.w;
        float iw2 = 1.0f / v2.w;

        v0.x *= iw0; v0.y *= iw0; v0.z *= iw0;
        v1.x *= iw1; v1.y *= iw1; v1.z *= iw1;
        v2.x *= iw2; v2.y *= iw2; v2.z *= iw2;

        // --- NDC - SCREEN ---
        Vec2 p0 = {(v0.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (v0.y + 1.0f) * 0.5f) * HEIGHT};
        Vec2 p1 = {(v1.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (v1.y + 1.0f) * 0.5f) * HEIGHT};
        Vec2 p2 = {(v2.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (v2.y + 1.0f) * 0.5f) * HEIGHT};

        float minXf = std::min({p0.x, p1.x, p2.x});
        float maxXf = std::max({p0.x, p1.x, p2.x});
        float minYf = std::min({p0.y, p1.y, p2.y});
        float maxYf = std::max({p0.y, p1.y, p2.y});

        int x0 = std::max(0, (int)minXf);
        int x1 = std::min(WIDTH - 1, (int)maxXf);
        int y0 = std::max(0, (int)minYf);
        int y1 = std::min(HEIGHT - 1, (int)maxYf);

        for (int yy = y0; yy <= y1; yy++)
        for (int xx = x0; xx <= x1; xx++)
        {
            Vec3 bc = barycentric(
                p0, p1, p2,
                {(float)xx + 0.5f, (float)yy + 0.5f}
            );

            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
                continue;

            int idx = yy * WIDTH + xx;

            // perspective-correct interpolation
            float w = bc.x * iw0 + bc.y * iw1 + bc.z * iw2;
            float invW = 1.0f / w;

            float z =
                (v0.z * bc.x * iw0 +
                 v1.z * bc.y * iw1 +
                 v2.z * bc.z * iw2) * invW;

            if (z < zbuffer[idx])
            {
                zbuffer[idx] = z;

                Vec3 color =
                    (tri.v0.color * (bc.x * iw0) +
                     tri.v1.color * (bc.y * iw1) +
                     tri.v2.color * (bc.z * iw2)) * invW;

                framebuffer[idx] = color;
            }
        }
    }
}
