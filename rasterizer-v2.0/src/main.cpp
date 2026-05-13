#include "math.h"
#include "render.h"
#include "geometry.h"
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
void renderFrame(HWND hwnd);

bool keys[256] = {};

const float EYE_HEIGHT = 1.6f;
const float PLAYER_HEIGHT = 1.8f;
const float PLAYER_RADIUS = 0.3f;


namespace Mouse{
    bool mouseInputCaptured = false;
    POINT lastMouse = {};
    const float MOUSE_SENS = 0.0025f;
}

RECT gClientRect;
POINT gCenterScreen;

struct Player {
    Vec3 pos = {2, 5, 2};
    Vec3 vel = {0,0,0};
    bool onGround = false;
} player;

bool solidAt(int x, int y, int z)
{
    for (const Chunk& c : chunks)
    {
        int lx = x - (int)c.origin.x;
        int ly = y - (int)c.origin.y;
        int lz = z - (int)c.origin.z;

        if (lx>=0 && ly>=0 && lz>=0 &&
            lx<CHUNK_SIZE && ly<CHUNK_SIZE && lz<CHUNK_SIZE)
        {
            return c.voxels[lx][ly][lz];
        }
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
    for (Chunk& c : chunks)
    {
        int lx = x - (int)c.origin.x;
        int ly = y - (int)c.origin.y;
        int lz = z - (int)c.origin.z;

        if (lx>=0 && ly>=0 && lz>=0 &&
            lx<CHUNK_SIZE && ly<CHUNK_SIZE && lz<CHUNK_SIZE)
        {
            c.voxels[lx][ly][lz] = value;
        }
    }
}

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

    chunks.push_back(createChunk({0,0,0}));

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
            renderFrame(hwnd);
        }
    }
    return 0;
}

void handleRay(bool place)
{
    Vec3 forward = {
        sinf(camYaw)*cosf(camPitch),
        -sinf(camPitch),
        cosf(camYaw)*cosf(camPitch)
    };

    Vec3 hit,normal;

    if (raycast(cameraPos,forward,6.0f,hit,normal))
    {
        if (place)
        {
            Vec3 p = hit + normal;
            modifyBlock((int)p.x,(int)p.y,(int)p.z,true);
        }
        else
        {
            modifyBlock((int)hit.x,(int)hit.y,(int)hit.z,false);
        }
    }
}

void renderFrame(HWND hwnd)
{
    std::fill(framebuffer,framebuffer+WIDTH*HEIGHT,Vec3{0,0,0});
    std::fill(zbuffer,zbuffer+WIDTH*HEIGHT,1e9f);

    Mat4 proj = perspective(90.0f*(3.14159f/180.0f),(float)WIDTH/HEIGHT,0.1f,100.0f);
    Mat4 view = rotateX(-camPitch)*rotateY(-camYaw)*translate(-cameraPos);

    int THREADS = std::max(1u,std::thread::hardware_concurrency());
    std::vector<std::thread> workers;

    int tileH = HEIGHT/THREADS;

    for(int i=0;i<THREADS;i++)
    {
        Tile t{0,WIDTH-1,i*tileH,(i==THREADS-1)?HEIGHT-1:(i+1)*tileH-1};
        workers.emplace_back(renderTile,t,view,proj);
    }

    for(auto& t:workers) t.join();

    unsigned char rgb[WIDTH*HEIGHT*3];
    for(int i=0;i<WIDTH*HEIGHT;i++)
    {
        rgb[i*3+0]=(unsigned char)(framebuffer[i].z*255);
        rgb[i*3+1]=(unsigned char)(framebuffer[i].y*255);
        rgb[i*3+2]=(unsigned char)(framebuffer[i].x*255);
    }

    HDC hdc=GetDC(hwnd);

    BITMAPINFO bmi={};
    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=WIDTH;
    bmi.bmiHeader.biHeight=-HEIGHT;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=24;

    StretchDIBits(hdc,0,0,WIDTH,HEIGHT,0,0,WIDTH,HEIGHT,rgb,&bmi,DIB_RGB_COLORS,SRCCOPY);
    ReleaseDC(hwnd,hdc);
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
