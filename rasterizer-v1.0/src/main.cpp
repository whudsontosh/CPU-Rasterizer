#include "math.h"
#include "render.h"
#include "geometry.h"
#include <windows.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <limits>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void renderFrame(HWND hwnd);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "RasterizerWindowClass";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, "CPU Rasterizer",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        WIDTH, HEIGHT, NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nShowCmd);

    chunks.push_back(createChunk({0,0,0}));
    // Main loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            renderFrame(hwnd);
        }
    }

    return 0;
}

void renderFrame(HWND hwnd)
{
    // Clear framebuffer & z-buffer
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        framebuffer[i] = {0,0,0};
        zbuffer[i] = std::numeric_limits<float>::infinity();
    }
    std::fill(zbuffer, zbuffer + WIDTH * HEIGHT, 1e9f);

    Mat4 proj = perspective(90.0f * (3.14159f/180.0f), (float)WIDTH/HEIGHT, 0.1f, 100.0f);
    Mat4 view = rotateX(-camPitch) * rotateY(-camYaw) * translate(-cameraPos);

    // Render only visible chunks and faces
    for (const Chunk& chunk : chunks)
        renderChunk(chunk, view, proj);

    // Blit framebuffer
    unsigned char rgbBuffer[WIDTH*HEIGHT*3];
    for (int i=0; i<WIDTH*HEIGHT; ++i)
    {
        rgbBuffer[i*3+0] = (unsigned char)(framebuffer[i].z*255.0f);
        rgbBuffer[i*3+1] = (unsigned char)(framebuffer[i].y*255.0f);
        rgbBuffer[i*3+2] = (unsigned char)(framebuffer[i].x*255.0f);
    }

    HDC hdc = GetDC(hwnd);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, rgbBuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        {
            const float speed = 0.9f;

            Vec3 forward = {
                sinf(camYaw) * cosf(camPitch),
                -sinf(camPitch),
                cosf(camYaw) * cosf(camPitch)
            };

            Vec3 right = {
                cosf(camYaw),
                0.0f,
                -sinf(camYaw)
            };

            switch (wParam)
            {
                case 'W':
                    cameraPos.x -= forward.x * speed;
                    cameraPos.y -= forward.y * speed;
                    cameraPos.z -= forward.z * speed;
                    break;

                case 'S':
                    cameraPos.x += forward.x * speed;
                    cameraPos.y += forward.y * speed;
                    cameraPos.z += forward.z * speed;
                    break;

                case 'A':
                    cameraPos.x -= right.x * speed;
                    cameraPos.z -= right.z * speed;
                    break;

                case 'D':
                    cameraPos.x += right.x * speed;
                    cameraPos.z += right.z * speed;
                    break;

                case VK_UP:
                    cameraPos.y += speed;
                    break;

                case VK_DOWN:
                    cameraPos.y -= speed;
                    break;
            }

            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }


        case WM_MOUSEMOVE:
        {
            if (!mouseCaptured) break;

            POINT p;
            GetCursorPos(&p);

            int dx = p.x - lastMouse.x;
            int dy = p.y - lastMouse.y;

            lastMouse = p;

            camYaw   += dx * MOUSE_SENS;
            camPitch += dy * MOUSE_SENS;

            // Clamp pitch so camera doesn't flip
            const float limit = 1.55f;
            if (camPitch > limit) camPitch = limit;
            if (camPitch < -limit) camPitch = -limit;

            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            SetCapture(hwnd);
            mouseCaptured = true;
            GetCursorPos(&lastMouse);
            return 0;
        }

        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            mouseCaptured = false;
            return 0;
        }

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
