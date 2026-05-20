#include "math.h"
#include <cmath>

// =====================
// Constants / Helpers
// =====================

static constexpr float EPSILON = 1e-6f;

// ---------Vec2---------

Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
Vec2 operator*(Vec2 v, float s) { return {v.x * s, v.y * s}; }

float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

float length(Vec2 v) { return std::sqrt(dot(v, v)); }

Vec2 normalize(Vec2 v) {
    float len = length(v);
    if (len < EPSILON) return {0, 0};
    return v * (1.0f / len);
}

// ---------Vec3---------

Vec3 operator-(const Vec3& v) {
    return { -v.x, -v.y, -v.z };
}

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(Vec3 v, float s) { return {v.x * s, v.y * s, v.z * s}; }
Vec3 operator/(Vec3 v, float s) { return {v.x / s, v.y / s, v.z / s}; }

float dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float length(Vec3 v) { return std::sqrt(dot(v, v)); }

Vec3 normalize(Vec3 v) {
    float len = length(v);
    if (len < EPSILON) return {0, 0, 0};
    return v * (1.0f / len);
}

Vec3 lerp(Vec3 a, Vec3 b, float t) {
    return a * (1.0f - t) + b * t;
}

// ---------Vec4---------

Vec4 operator+(Vec4 a, Vec4 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

Vec4 operator*(Vec4 v, float s) {
    return {v.x * s, v.y * s, v.z * s, v.w * s};
}

Vec4 operator/(Vec4 v, float s) {
    return {v.x / s, v.y / s, v.z / s, v.w / s};
}

// ---------Mat4---------

Mat4 identity() {
    Mat4 m = {};
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

Mat4 translate(Vec3 t) {
    Mat4 m = identity();
    m.m[12] = t.x;
    m.m[13] = t.y;
    m.m[14] = t.z;
    return m;
}

Mat4 scale(Vec3 s) {
    Mat4 m = identity();
    m.m[0] = s.x;
    m.m[5] = s.y;
    m.m[10] = s.z;
    return m;
}

Mat4 rotateX(float r) {
    Mat4 m = identity();
    float c = std::cos(r);
    float s = std::sin(r);
    m.m[5] = c;   m.m[9]  = -s;
    m.m[6] = s;   m.m[10] = c;
    return m;
}

Mat4 rotateY(float r) {
    Mat4 m = identity();
    float c = std::cos(r);
    float s = std::sin(r);
    m.m[0] = c;   m.m[8]  = s;
    m.m[2] = -s;  m.m[10] = c;
    return m;
}

Mat4 rotateZ(float r) {
    Mat4 m = identity();
    float c = std::cos(r);
    float s = std::sin(r);
    m.m[0] = c;   m.m[4] = -s;
    m.m[1] = s;   m.m[5] = c;
    return m;
}

// FIXED: column-major matrix multiplication
Mat4 operator*(Mat4 a, Mat4 b) {
    Mat4 r = {};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            for (int i = 0; i < 4; ++i) {
                r.m[col * 4 + row] +=
                    a.m[i * 4 + row] * b.m[col * 4 + i];
            }
        }
    }
    return r;
}

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = normalize(center - eye);
    Vec3 r = normalize(cross(f, up));
    Vec3 u = cross(r, f);

    Mat4 m = identity();

    m.m[0] = r.x;  m.m[4] = r.y;  m.m[8]  = r.z;
    m.m[1] = u.x;  m.m[5] = u.y;  m.m[9]  = u.z;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z;

    m.m[12] = -dot(r, eye);
    m.m[13] = -dot(u, eye);
    m.m[14] =  dot(f, eye);

    return m;
}

Mat4 perspective(float fov, float aspect, float nearP, float farP) {
    float f = 1.0f / std::tan(fov * 0.5f);
    Mat4 m = {};

    m.m[0] = f / aspect;
    m.m[5] = f;
    m.m[10] = (farP + nearP) / (nearP - farP);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * farP * nearP) / (nearP - farP);

    return m;
}

// ---------Mat4 & Vec4---------

Vec4 operator*(Mat4 m, Vec4 v) {
    return {
        m.m[0]*v.x + m.m[4]*v.y + m.m[8]*v.z  + m.m[12]*v.w,
        m.m[1]*v.x + m.m[5]*v.y + m.m[9]*v.z  + m.m[13]*v.w,
        m.m[2]*v.x + m.m[6]*v.y + m.m[10]*v.z + m.m[14]*v.w,
        m.m[3]*v.x + m.m[7]*v.y + m.m[11]*v.z + m.m[15]*v.w
    };
}

// =====================
// Rasterizer helpers
// =====================

// Barycentric coordinates (screen-space)
Vec3 barycentric(Vec2 a, Vec2 b, Vec2 c, Vec2 p) {
    Vec2 v0 = b - a;
    Vec2 v1 = c - a;
    Vec2 v2 = p - a;

    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    if (std::fabs(denom) < EPSILON)
        return {-1, -1, -1};

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return {u, v, w};
}
