#pragma once

#include <array>
#include <cassert>
#include <cmath>

namespace math {

struct Vec4 {
    float x;
    float y;
    float z;
    float w;

    Vec4 operator+(const Vec4& other) const {
        return {
            .x = x + other.x,
            .y = y + other.y,
            .z = z + other.z,
            .w = w + other.w,
        };
    }

    Vec4& operator+=(const Vec4& other) {
        return *this = *this + other;
    }

    Vec4 operator-(const Vec4& other) const {
        return {
            .x = x - other.x,
            .y = y - other.y,
            .z = z - other.z,
            .w = w - other.w,
        };
    }

    Vec4& operator-=(const Vec4& other) {
        return *this = *this - other;
    }

    bool operator==(const Vec4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    Vec4 operator-() const {
        return {
            .x = -x,
            .y = -y,
            .z = -z,
            .w = -w,
        };
    }

    float Len() {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    void Normalize() {
        auto len = Len();
        assert(len > 1e-9);
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }

    Vec4 Normalized() const {
        auto copy = *this;
        copy.Normalize();
        return copy;
    }

    static float Dot(const Vec4& a, const Vec4& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    float& operator[](size_t i) {
        if (i == 0) {
            return x;
        }
        if (i == 1) {
            return y;
        }
        if (i == 2) {
            return z;
        }
        if (i == 3) {
            return w;
        }
        assert(false);
    }

    float operator[](size_t i) const {
        if (i == 0) {
            return x;
        }
        if (i == 1) {
            return y;
        }
        if (i == 2) {
            return z;
        }
        if (i == 3) {
            return w;
        }
        assert(false);
    }
};

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3 operator+(const Vec3& other) const {
        return {
            .x = x + other.x,
            .y = y + other.y,
            .z = z + other.z,
        };
    }

    Vec3& operator+=(const Vec3& other) {
        return *this = *this + other;
    }

    Vec3 operator-(const Vec3& other) const {
        return {
            .x = x - other.x,
            .y = y - other.y,
            .z = z - other.z,
        };
    }

    Vec3& operator-=(const Vec3& other) {
        return *this = *this - other;
    }

    bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    Vec3 operator-() const {
        return {
            .x = -x,
            .y = -y,
            .z = -z,
        };
    }

    inline math::Vec3 operator*(float f) const {
        return {f * x, f * y, f * z};
    }

    float Len() {
        return std::sqrt(x * x + y * y + z * z);
    }

    void Normalize() {
        auto len = Len();
        assert(len > 1e-9);
        x /= len;
        y /= len;
        z /= len;
    }

    Vec3 Normalized() const {
        auto copy = *this;
        copy.Normalize();
        return copy;
    }

    static Vec3 Cross(const Vec3& a, const Vec3& b) {
        return {
            .x = a.y * b.z - a.z * b.y,
            .y = a.z * b.x - a.x * b.z,
            .z = a.x * b.y - a.y * b.x,
        };
    }

    static float Dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

struct Vec2 {
    float x;
    float y;

    Vec2 operator-(const Vec2& other) const {
        return {
            .x = x - other.x,
            .y = y - other.y,
        };
    }

    Vec2 operator+(const Vec2& other) const {
        return {
            .x = x + other.x,
            .y = y + other.y,
        };
    }

    Vec2 operator*(float c) const {
        return {.x = x * c, .y = y * c};
    }

    static float Dot(const Vec2& a, const Vec2& b) {
        return a.x * b.x + a.y * b.y;
    }
};

using Mat4 = std::array<std::array<float, 4>, 4>;

inline auto EmtpyMat4() {
    Mat4 result;
    result[0] = {};
    result[1] = {};
    result[2] = {};
    result[3] = {};
    return result;
};

inline auto IdentityMat4() {
    Mat4 result;
    result[0] = {};
    result[1] = {};
    result[2] = {};
    result[3] = {};
    result[0][0] = 1.0;
    result[1][1] = 1.0;
    result[2][2] = 1.0;
    result[3][3] = 1.0;
    return result;
};

}  // namespace math

inline math::Mat4 operator*(const math::Mat4& a, const math::Mat4& b) {
    math::Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = 0.0;
            for (int k = 0; k < 4; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

inline math::Vec4 operator*(const math::Mat4& a, const math::Vec4& x) {
    math::Vec4 result = {};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i] += a[i][j] * x[j];
        }
    }
    return result;
}

inline math::Vec3 operator*(float f, const math::Vec3& x) {
    return {f * x.x, f * x.y, f * x.z};
}

inline math::Vec4 operator*(float f, const math::Vec4& x) {
    return {f * x.x, f * x.y, f * x.z, f * x.w};
}

inline math::Vec4 operator*(const math::Vec4& x, float f) {
    return {f * x[0], f * x[1], f * x[2], f * x[3]};
}

namespace math {

template<typename V>
V Blend(V first, V second, float ratio) {
    return (1 - ratio) * first + ratio * second;
}

}  // namespace math

