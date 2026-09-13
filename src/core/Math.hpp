#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace Aetheria {

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

struct Vec2 {
    float x = 0.0f, y = 0.0f;
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(lengthSq()); }
    Vec3 normalized() const {
        float l = length();
        return l > 0.00001f ? *this / l : Vec3(0, 0, 0);
    }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        };
    }
};

struct Vec4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;
    Vec4() = default;
    Vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    Vec4 operator+(const Vec4& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    Vec4 operator-(const Vec4& o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w}; }
    Vec4 operator*(const Vec4& o) const { return {x * o.x, y * o.y, z * o.z, w * o.w}; }
    friend Vec4 operator*(float s, const Vec4& v) { return v * s; }
};

struct IVec3 {
    int32_t x = 0, y = 0, z = 0;
    IVec3() = default;
    IVec3(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}
    bool operator==(const IVec3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const IVec3& o) const { return !(*this == o); }
    IVec3 operator+(const IVec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    IVec3 operator-(const IVec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
};

struct Mat4 {
    float m[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    static Mat4 identity() {
        return Mat4();
    }

    static Mat4 translation(const Vec3& v) {
        Mat4 res;
        res.m[12] = v.x;
        res.m[13] = v.y;
        res.m[14] = v.z;
        return res;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 res;
        res.m[0] = s.x;
        res.m[5] = s.y;
        res.m[10] = s.z;
        return res;
    }

    static Mat4 rotationY(float rad) {
        Mat4 res;
        float c = std::cos(rad);
        float s = std::sin(rad);
        res.m[0] = c;
        res.m[2] = -s;
        res.m[8] = s;
        res.m[10] = c;
        return res;
    }

    static Mat4 rotationX(float rad) {
        Mat4 res;
        float c = std::cos(rad);
        float s = std::sin(rad);
        res.m[5] = c;
        res.m[6] = s;
        res.m[9] = -s;
        res.m[10] = c;
        return res;
    }

    static Mat4 rotationZ(float rad) {
        Mat4 res;
        float c = std::cos(rad);
        float s = std::sin(rad);
        res.m[0] = c;
        res.m[1] = s;
        res.m[4] = -s;
        res.m[5] = c;
        return res;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res.m[c * 4 + r] =
                    m[0 * 4 + r] * o.m[c * 4 + 0] +
                    m[1 * 4 + r] * o.m[c * 4 + 1] +
                    m[2 * 4 + r] * o.m[c * 4 + 2] +
                    m[3 * 4 + r] * o.m[c * 4 + 3];
            }
        }
        return res;
    }

    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        );
    }

    // Vulkan Clip Space (Y is down, Z in [0, 1])
    static Mat4 perspectiveVK(float fovYRad, float aspect, float zNear, float zFar) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        float f = 1.0f / std::tan(fovYRad * 0.5f);
        res.m[0] = f / aspect;
        res.m[5] = -f; // Vulkan Y-down flip
        res.m[10] = zFar / (zNear - zFar);
        res.m[11] = -1.0f;
        res.m[14] = -(zFar * zNear) / (zFar - zNear);
        return res;
    }

    // OpenGL Clip Space (Y is up, Z in [-1, 1])
    static Mat4 perspectiveGL(float fovYRad, float aspect, float zNear, float zFar) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        float f = 1.0f / std::tan(fovYRad * 0.5f);
        res.m[0] = f / aspect;
        res.m[5] = f;
        res.m[10] = -(zFar + zNear) / (zFar - zNear);
        res.m[11] = -1.0f;
        res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
        return res;
    }

    static Mat4 orthoVK(float left, float right, float bottom, float top, float zNear, float zFar) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        res.m[0] = 2.0f / (right - left);
        res.m[5] = -2.0f / (bottom - top);
        res.m[10] = 1.0f / (zFar - zNear);
        res.m[12] = -(right + left) / (right - left);
        res.m[13] = -(bottom + top) / (bottom - top);
        res.m[14] = -zNear / (zFar - zNear);
        res.m[15] = 1.0f;
        return res;
    }

    static Mat4 orthoGL(float left, float right, float bottom, float top, float zNear = -1.0f, float zFar = 1.0f) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        res.m[0] = 2.0f / (right - left);
        res.m[5] = 2.0f / (top - bottom);
        res.m[10] = -2.0f / (zFar - zNear);
        res.m[12] = -(right + left) / (right - left);
        res.m[13] = -(top + bottom) / (top - bottom);
        res.m[14] = -(zFar + zNear) / (zFar - zNear);
        res.m[15] = 1.0f;
        return res;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 res;
        res.m[0] = s.x;
        res.m[4] = s.y;
        res.m[8] = s.z;
        res.m[12] = -s.dot(eye);

        res.m[1] = u.x;
        res.m[5] = u.y;
        res.m[9] = u.z;
        res.m[13] = -u.dot(eye);

        res.m[2] = -f.x;
        res.m[6] = -f.y;
        res.m[10] = -f.z;
        res.m[14] = f.dot(eye);

        res.m[3] = 0.0f;
        res.m[7] = 0.0f;
        res.m[11] = 0.0f;
        res.m[15] = 1.0f;
        return res;
    }

    Mat4 inverse() const {
        Mat4 inv;
        float s0 = m[0] * m[5] - m[4] * m[1];
        float s1 = m[0] * m[6] - m[4] * m[2];
        float s2 = m[0] * m[7] - m[4] * m[3];
        float s3 = m[1] * m[6] - m[5] * m[2];
        float s4 = m[1] * m[7] - m[5] * m[3];
        float s5 = m[2] * m[7] - m[6] * m[3];

        float c5 = m[10] * m[15] - m[14] * m[11];
        float c4 = m[9] * m[15] - m[13] * m[11];
        float c3 = m[9] * m[14] - m[13] * m[10];
        float c2 = m[8] * m[15] - m[12] * m[11];
        float c1 = m[8] * m[14] - m[12] * m[10];
        float c0 = m[8] * m[13] - m[12] * m[9];

        float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
        if (std::abs(det) < 1e-8f) return Mat4::identity();

        float invDet = 1.0f / det;

        inv.m[0] = (m[5] * c5 - m[6] * c4 + m[7] * c3) * invDet;
        inv.m[1] = (-m[1] * c5 + m[2] * c4 - m[3] * c3) * invDet;
        inv.m[2] = (m[13] * s5 - m[14] * s4 + m[15] * s3) * invDet;
        inv.m[3] = (-m[9] * s5 + m[10] * s4 - m[11] * s3) * invDet;

        inv.m[4] = (-m[4] * c5 + m[6] * c2 - m[7] * c1) * invDet;
        inv.m[5] = (m[0] * c5 - m[2] * c2 + m[3] * c1) * invDet;
        inv.m[6] = (-m[12] * s5 + m[14] * s2 - m[15] * s1) * invDet;
        inv.m[7] = (m[8] * s5 - m[10] * s2 + m[11] * s1) * invDet;

        inv.m[8] = (m[4] * c4 - m[5] * c2 + m[7] * c0) * invDet;
        inv.m[9] = (-m[0] * c4 + m[1] * c2 - m[3] * c0) * invDet;
        inv.m[10] = (m[12] * s4 - m[13] * s2 + m[15] * s0) * invDet;
        inv.m[11] = (-m[8] * s4 + m[9] * s2 - m[11] * s0) * invDet;

        inv.m[12] = (-m[4] * c3 + m[5] * c1 - m[6] * c0) * invDet;
        inv.m[13] = (m[0] * c3 - m[1] * c1 + m[2] * c0) * invDet;
        inv.m[14] = (-m[12] * s3 + m[13] * s1 - m[14] * s0) * invDet;
        inv.m[15] = (m[8] * s3 - m[9] * s1 + m[10] * s0) * invDet;

        return inv;
    }
};

struct AABB {
    Vec3 min;
    Vec3 max;

    AABB() : min(0, 0, 0), max(0, 0, 0) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}

    // Strict intersection (open intervals): zero-thickness contact is not an overlap
    bool intersects(const AABB& o) const {
        return (min.x < o.max.x && max.x > o.min.x) &&
               (min.y < o.max.y && max.y > o.min.y) &&
               (min.z < o.max.z && max.z > o.min.z);
    }

    // Inclusive intersection (closed intervals)
    bool intersectsInclusive(const AABB& o) const {
        return (min.x <= o.max.x && max.x >= o.min.x) &&
               (min.y <= o.max.y && max.y >= o.min.y) &&
               (min.z <= o.max.z && max.z >= o.min.z);
    }

    bool contains(const Vec3& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }

    AABB offset(const Vec3& off) const {
        return AABB(min + off, max + off);
    }

    AABB expand(float dx, float dy, float dz) const {
        Vec3 newMin = min;
        Vec3 newMax = max;
        if (dx < 0.0f) newMin.x += dx;
        if (dx > 0.0f) newMax.x += dx;
        if (dy < 0.0f) newMin.y += dy;
        if (dy > 0.0f) newMax.y += dy;
        if (dz < 0.0f) newMin.z += dz;
        if (dz > 0.0f) newMax.z += dz;
        return AABB(newMin, newMax);
    }

    // Swept X-axis offset calculation against another AABB
    float calculateXOffset(const AABB& other, float offset) const {
        if (other.max.y <= min.y || other.min.y >= max.y) return offset;
        if (other.max.z <= min.z || other.min.z >= max.z) return offset;
        if (offset > 0.0f && other.min.x >= max.x) {
            float d = (other.min.x - max.x) - 0.0001f;
            if (d < 0.0f) d = 0.0f;
            if (d < offset) offset = d;
        } else if (offset < 0.0f && other.max.x <= min.x) {
            float d = (other.max.x - min.x) + 0.0001f;
            if (d > 0.0f) d = 0.0f;
            if (d > offset) offset = d;
        }
        return offset;
    }

    // Swept Y-axis offset calculation against another AABB
    float calculateYOffset(const AABB& other, float offset) const {
        if (other.max.x <= min.x || other.min.x >= max.x) return offset;
        if (other.max.z <= min.z || other.min.z >= max.z) return offset;
        if (offset > 0.0f && other.min.y >= max.y) {
            float d = (other.min.y - max.y) - 0.0001f;
            if (d < 0.0f) d = 0.0f;
            if (d < offset) offset = d;
        } else if (offset < 0.0f && other.max.y <= min.y) {
            float d = (other.max.y - min.y);
            if (d > offset) offset = d;
        }
        return offset;
    }

    // Swept Z-axis offset calculation against another AABB
    float calculateZOffset(const AABB& other, float offset) const {
        if (other.max.x <= min.x || other.min.x >= max.x) return offset;
        if (other.max.y <= min.y || other.min.y >= max.y) return offset;
        if (offset > 0.0f && other.min.z >= max.z) {
            float d = (other.min.z - max.z) - 0.0001f;
            if (d < 0.0f) d = 0.0f;
            if (d < offset) offset = d;
        } else if (offset < 0.0f && other.max.z <= min.z) {
            float d = (other.max.z - min.z) + 0.0001f;
            if (d > 0.0f) d = 0.0f;
            if (d > offset) offset = d;
        }
        return offset;
    }
};

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray(const Vec3& origin, const Vec3& direction)
        : origin(origin), direction(direction.normalized()) {}

    Vec3 pointAt(float t) const {
        return origin + direction * t;
    }
};

} // namespace Aetheria
