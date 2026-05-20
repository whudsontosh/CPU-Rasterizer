#include "math.h"
#include "render.h"
#include "geometry.h"
#include "texture.h"
#include "physics.h"
#include "ray.h"
#include "world.h"
#include <windows.h>
#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
#include <string>
#include <iostream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool keys[256] = {};
Player player;

namespace Mouse{
    bool mouseInputCaptured = false;
    POINT lastMouse = {};
    const float MOUSE_SENS = 0.0025f;
}

RECT gClientRect = {};
POINT gCenterScreen = {};

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int nShowCmd)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "RasterizerWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0,wc.lpszClassName,"Voxel",
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
        WIDTH,HEIGHT,NULL,NULL,hInstance,NULL);

    ShowWindow(hwnd,nShowCmd);

    // Mouse Input
    GetClientRect(hwnd, &gClientRect);

    POINT centerClient = {
        (gClientRect.right - gClientRect.left) / 2,
        (gClientRect.bottom - gClientRect.top) / 2
    };

    gCenterScreen = centerClient;
    ClientToScreen(hwnd, &gCenterScreen);

    SetCursorPos(gCenterScreen.x, gCenterScreen.y);
    Mouse::mouseInputCaptured = true;
    ShowCursor(FALSE);

    // Texture Loading
    baseTexture = TEX::loadBMP("assets/Tex1.bmp");

    chunks.emplace(ChunkCoord{0,0,0}, createChunk({0,0,0}, true));
    //chunks.push_back(createChunk({CHUNK_SIZE,0,0}));
    //chunks.push_back(createChunk({0,CHUNK_SIZE,0}, true));

    // --- SAFE SPAWN ---
    int sx = 2;
    int sz = 2;

    int groundY = findGroundY(sx, sz);

    // fallback if no ground found
    if (groundY < 0) groundY = 5;

    player.pos = {
        (float)sx + 0.5f,
        (float)groundY + 1.5f, // important: above ground + player height buffer
        (float)sz + 0.5f
    };

    cameraPos = player.pos + Vec3{0, EYE_HEIGHT, 0};

    MSG msg = {};

    LARGE_INTEGER freq,prev,curr;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            QueryPerformanceCounter(&curr);
            float dt = (float)(curr.QuadPart-prev.QuadPart)/freq.QuadPart;
            prev = curr;

            updatePlayer(dt);
            updateChunks(player.pos);
            renderFrame(hwnd);
        }
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY: PostQuitMessage(0); return 0;

        case WM_KEYDOWN:
            {
                keys[wParam] = true;

                if (wParam == VK_ESCAPE)
                {
                    Mouse::mouseInputCaptured = !Mouse::mouseInputCaptured;

                    if (Mouse::mouseInputCaptured)
                    {
                        SetCursorPos(gCenterScreen.x, gCenterScreen.y);
                        ShowCursor(FALSE);
                    }
                    else
                    {
                        ShowCursor(TRUE);
                    }
                }

                return 0;
            }

        case WM_KEYUP:   keys[wParam]=false; return 0;

        case WM_MOUSEMOVE:
        {
            if (!Mouse::mouseInputCaptured)
                break;

            POINT p;
            GetCursorPos(&p);

            int dx = p.x - gCenterScreen.x;
            int dy = p.y - gCenterScreen.y;

            camYaw   -= dx * Mouse::MOUSE_SENS;
            camPitch -= dy * Mouse::MOUSE_SENS;

            camPitch = std::clamp(camPitch, -1.55f, 1.55f);

            SetCursorPos(gCenterScreen.x, gCenterScreen.y);

            return 0;
        }

        case WM_LBUTTONDOWN: handleRay(false); return 0; // break
        case WM_RBUTTONDOWN: handleRay(true);  return 0; // place
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}
