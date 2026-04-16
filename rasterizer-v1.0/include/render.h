#pragma once
#include "math.h"
#include "geometry.h"
#include <windows.h>
#include <vector>

// GLOBALS (DECLARATIONS ONLY)

extern const int WIDTH;
extern const int HEIGHT;

extern Vec3 cameraPos;
extern float camYaw;
extern float camPitch;

extern POINT lastMouse;
extern bool mouseCaptured;

extern const float MOUSE_SENS;

extern std::vector<Chunk> chunks;

extern Vec3 framebuffer[];
extern float zbuffer[];

// FUNCTION PROTOTYPES
bool chunkInFrustum(const Chunk& chunk, const Mat4& view, const Mat4& proj);
void renderChunk(const Chunk& chunk, const Mat4& view, const Mat4& proj);
