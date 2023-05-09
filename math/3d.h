#pragma once

#include "common.h"
#include <iostream>

namespace math {

inline Mat4 Perspective(
    float fovy,
    float aspect,
    float zNear,
    float zFar)
{
    assert(aspect != 0.0);
    assert(zFar != zNear);

    float tanHalfFovy = tan(fovy / 2.0);

    Mat4 result = EmtpyMat4();
    result[0][0] = 1.0 / (aspect * tanHalfFovy);
    result[1][1] = 1.0 / (tanHalfFovy);
    result[2][2] = -(zFar + zNear) / (zFar - zNear);
    result[3][2] = -1.0;
    result[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
    return result;
}

inline Mat4 LookAt(Vec3 position, Vec3 target, Vec3 worldUp)
{
    Vec3 zaxis = (position - target).Normalized();
    Vec3 xaxis = (Vec3::Cross(worldUp.Normalized(), zaxis));
    Vec3 yaxis = Vec3::Cross(zaxis, xaxis);

    Mat4 translation = IdentityMat4();
    translation[0][3] = -position.x;
    translation[1][3] = -position.y;
    translation[2][3] = -position.z;
    Mat4 rotation = IdentityMat4();
    rotation[0][0] = xaxis.x;
    rotation[0][1] = xaxis.y;
    rotation[0][2] = xaxis.z;
    rotation[1][0] = yaxis.x;
    rotation[1][1] = yaxis.y;
    rotation[1][2] = yaxis.z;
    rotation[2][0] = zaxis.x;
    rotation[2][1] = zaxis.y;
    rotation[2][2] = zaxis.z; 
    return rotation * translation;
}

inline Mat4 Rotate(
    float angle,
    Vec3 const & v)
{
    float a = angle;
    float c = cos(a);
    float s = sin(a);

    Vec3 axis = v.Normalized();
    Vec3 temp = (1.0 - c) * axis;

    Mat4 rotate = IdentityMat4();
    rotate[0][0] = c + temp.x * axis.x;
    rotate[1][0] = 0 + temp.x * axis.y + s * axis.z;
    rotate[2][0] = 0 + temp.x * axis.z - s * axis.y;

    rotate[0][1] = 0 + temp.y * axis.x - s * axis.z;
    rotate[1][1] = c + temp.y * axis.y;
    rotate[2][1] = 0 + temp.y * axis.z + s * axis.x;

    rotate[0][2] = 0 + temp.z * axis.x + s * axis.y;
    rotate[1][2] = 0 + temp.z * axis.y - s * axis.x;
    rotate[2][2] = c + temp.z * axis.z;
    return rotate;
}

inline Vec3 FromProjective(const Vec4& vec) {
    return {
        vec[0] / vec[3],
        vec[1] / vec[3],
        vec[2] / vec[3],
    };
}

}  // namespace math

