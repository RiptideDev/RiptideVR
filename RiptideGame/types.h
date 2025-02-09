#pragma once
#include <string>
#include <cmath>

struct Asset
{
    int assetid;
    std::string file_name;

    void* content;
    int len;
    bool loaded;
};

struct Vector2
{
    Vector2() { x = 0; y = 0; }
    Vector2(float xx) { x = xx; y = xx; }
    Vector2(float xx, float yy) { x = xx; y = yy; }

    Vector2 operator+(const Vector2& v) const { return { x + v.x, y + v.y }; }
    Vector2 operator-(const Vector2& v) const { return { x - v.x, y - v.y }; }
    Vector2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
    Vector2 operator/(float scalar) const { return { x / scalar, y / scalar }; }

    float Magnitude() const { return std::sqrt(x * x + y * y); }
    Vector2 Normalized() const { float mag = Magnitude(); return (mag > 0) ? (*this / mag) : Vector2(); }

    float x, y;
};

struct Vector3
{
    Vector3() { x = 0; y = 0; z = 0; }
    Vector3(float xx) { x = xx; y = xx; z = xx; }
    Vector3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }

    Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    Vector3 operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }

    float Magnitude() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 Normalized() const { float mag = Magnitude(); return (mag > 0) ? (*this / mag) : Vector3(); }

    float x, y, z;
};
