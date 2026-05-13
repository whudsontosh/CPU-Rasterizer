#pragma once

// =====================
// Basic Types
// =====================

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat4 { float m[16]; };

// =====================
// Vec2
// =====================

Vec3 operator-(const Vec3& v);

Vec2 operator+(Vec2 a, Vec2 b);
Vec2 operator-(Vec2 a, Vec2 b);
Vec2 operator*(Vec2 v, float s);

float dot(Vec2 a, Vec2 b);
float length(Vec2 v);
Vec2 normalize(Vec2 v);

// =====================
// Vec3
// =====================

Vec3 operator+(Vec3 a, Vec3 b);
Vec3 operator-(Vec3 a, Vec3 b);
Vec3 operator*(Vec3 v, float s);
Vec3 operator/(Vec3 v, float s);

float dot(Vec3 a, Vec3 b);
Vec3 cross(Vec3 a, Vec3 b);

float length(Vec3 v);
Vec3 normalize(Vec3 v);

Vec3 lerp(Vec3 a, Vec3 b, float t);

// =====================
// Vec4
// =====================

Vec4 operator+(Vec4 a, Vec4 b);
Vec4 operator*(Vec4 v, float s);
Vec4 operator/(Vec4 v, float s);

// =====================
// Mat4
// =====================

Mat4 identity();

Mat4 translate(Vec3 t);
Mat4 scale(Vec3 s);

Mat4 rotateX(float radians);
Mat4 rotateY(float radians);
Mat4 rotateZ(float radians);

Mat4 operator*(Mat4 a, Mat4 b);

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up);
Mat4 perspective(float fov, float aspect, float nearP, float farP);

// =====================
// Mat4 & Vec4
// =====================

Vec4 operator*(Mat4 m, Vec4 v);

// =====================
// Rasterizer Helpers
// =====================

// Returns (u, v, w). If any < 0  outside triangle
Vec3 barycentric(Vec2 a, Vec2 b, Vec2 c, Vec2 p);
