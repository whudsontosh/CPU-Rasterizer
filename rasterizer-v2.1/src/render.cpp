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

void renderTile(const Tile& tile, const Mat4& view, const Mat4& proj)
{
    for (const Chunk& chunk : chunks)
    {
        for (const Triangle& tri : chunk.triangles)
        {
            // --- WORLD to VIEW ---
            Vec4 v0v = view * Vec4{tri.v0.position.x, tri.v0.position.y, tri.v0.position.z, 1};
            Vec4 v1v = view * Vec4{tri.v1.position.x, tri.v1.position.y, tri.v1.position.z, 1};
            Vec4 v2v = view * Vec4{tri.v2.position.x, tri.v2.position.y, tri.v2.position.z, 1};

            Vec3 v0_view = { v0v.x, v0v.y, v0v.z };
            Vec3 v1_view = { v1v.x, v1v.y, v1v.z };
            Vec3 v2_view = { v2v.x, v2v.y, v2v.z };

            const float nearCam = 0.000001f; // This value controls the padding between the camera and faces

            if (v0_view.z > -nearCam || v1_view.z > -nearCam || v2_view.z > -nearCam)
                continue;


            // --- BACKFACE CULL (VIEW SPACE) ---
            Vec3 e1 = v1_view - v0_view;
            Vec3 e2 = v2_view - v0_view;
            Vec3 normal = cross(e1, e2);

            Vec3 center = (v0_view + v1_view + v2_view) * (1.0f / 3.0f);
            Vec3 viewDir = -center;

            if (dot(normal, viewDir) >= 0.0f)
                continue;

            // --- VIEW to CLIP ---
            Vec4 v0c = proj * v0v;
            Vec4 v1c = proj * v1v;
            Vec4 v2c = proj * v2v;

            // --- CLIP REJECTION ---
            auto outside = [](Vec4 a, Vec4 b, Vec4 c, int axis)
            {
                if (axis == 0)
                    return (a.x < -a.w && b.x < -b.w && c.x < -c.w) ||
                           (a.x >  a.w && b.x >  b.w && c.x >  c.w);

                if (axis == 1)
                    return (a.y < -a.w && b.y < -b.w && c.y < -c.w) ||
                           (a.y >  a.w && b.y >  b.w && c.y >  c.w);

                return (a.z < -a.w && b.z < -b.w && c.z < -c.w) ||
                       (a.z >  a.w && b.z >  b.w && c.z >  c.w);
            };

            if (outside(v0c, v1c, v2c, 0) ||
                outside(v0c, v1c, v2c, 1) ||
                outside(v0c, v1c, v2c, 2))
                continue;

            // --- PERSPECTIVE DIVIDE ---
            float iw0 = 1.0f / v0c.w;
            float iw1 = 1.0f / v1c.w;
            float iw2 = 1.0f / v2c.w;

            Vec3 ndc0 = { v0c.x * iw0, v0c.y * iw0, v0c.z * iw0 };
            Vec3 ndc1 = { v1c.x * iw1, v1c.y * iw1, v1c.z * iw1 };
            Vec3 ndc2 = { v2c.x * iw2, v2c.y * iw2, v2c.z * iw2 };

            // --- SCREEN SPACE ---
            Vec2 p0 = {(ndc0.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (ndc0.y + 1.0f) * 0.5f) * HEIGHT};
            Vec2 p1 = {(ndc1.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (ndc1.y + 1.0f) * 0.5f) * HEIGHT};
            Vec2 p2 = {(ndc2.x + 1.0f) * 0.5f * WIDTH,  (1.0f - (ndc2.y + 1.0f) * 0.5f) * HEIGHT};

            float minXf = std::min({p0.x, p1.x, p2.x});
            float maxXf = std::max({p0.x, p1.x, p2.x});
            float minYf = std::min({p0.y, p1.y, p2.y});
            float maxYf = std::max({p0.y, p1.y, p2.y});

            int x0 = std::max(tile.x0, (int)minXf);
            int x1 = std::min(tile.x1, (int)maxXf);
            int y0 = std::max(tile.y0, (int)minYf);
            int y1 = std::min(tile.y1, (int)maxYf);

            // --- RASTER ---
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

                // --- PERSPECTIVE CORRECT ---
                float w = bc.x * iw0 + bc.y * iw1 + bc.z * iw2;
                float invW = 1.0f / w;

                // --- DEPTH (NDC SPACE, CORRECT) ---
                float z =
                    (ndc0.z * bc.x * iw0 +
                     ndc1.z * bc.y * iw1 +
                     ndc2.z * bc.z * iw2) * invW;

                // optional clamp (helps stability)
                if (z < -1.0f || z > 1.0f)
                    continue;

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
}
