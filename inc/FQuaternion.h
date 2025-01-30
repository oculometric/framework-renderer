#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "FVector.h"

class FQuaternion
{
public:
    FVector    i = FVector(0.0f, 0.0f, 0.0f);   // vector part: v.x, v.y, v.z
    float      r = 1.0f;                        // number (scalar) part

    FQuaternion();
    FQuaternion(float e0, float e1, float e2, float e3);

    float magnitude();
    FVector getVector();
    float getScalar();
    DirectX::XMFLOAT4 getDirectXRep();
    FQuaternion operator+=(FQuaternion q);
    FQuaternion operator-=(FQuaternion q);
    FQuaternion operator*=(float s);
    FQuaternion operator/=(float s);
    FQuaternion operator~() const;
};

inline DirectX::XMFLOAT4 FQuaternion::getDirectXRep()
{
    return DirectX::XMFLOAT4(i.x, i.y, i.z, r);
}

inline FQuaternion::FQuaternion()
{
    r = 1;
    i.x = 0;
    i.y = 0;
    i.z = 0;
}

inline FQuaternion::FQuaternion(float e0, float e1, float e2, float e3)
{
    r = e0;
    i.x = e1;
    i.y = e2;
    i.z = e3;
}

inline float FQuaternion::magnitude(void)
{
    return (float)sqrt(r * r + i.x * i.x + i.y * i.y + i.z * i.z);
}

inline FVector FQuaternion::getVector(void)
{
    return FVector(i.x, i.y, i.z);
}

inline float FQuaternion::getScalar(void)
{
    return r;
}

inline FQuaternion FQuaternion::operator+=(FQuaternion q)
{
    r += q.r;
    i.x += q.i.x;
    i.y += q.i.y;
    i.z += q.i.z;
    return *this;
}

inline FQuaternion FQuaternion::operator-=(FQuaternion q)
{
    r -= q.r;
    i.x -= q.i.x;
    i.y -= q.i.y;
    i.z -= q.i.z;
    return *this;
}

inline FQuaternion FQuaternion::operator*=(float s)
{
    r *= s;
    i.x *= s;
    i.y *= s;
    i.z *= s;
    return *this;
}

inline FQuaternion FQuaternion::operator/=(float s)
{
    r /= s;
    i.x /= s;
    i.y /= s;
    i.z /= s;
    return *this;
}

inline FQuaternion FQuaternion::operator~(void) const
{
    return FQuaternion(r, -i.x, -i.y, -i.z);
}

inline FQuaternion operator+(FQuaternion q1, FQuaternion q2)
{
    return FQuaternion(q1.r + q2.r, q1.i.x + q2.i.x, q1.i.y + q2.i.y, q1.i.z + q2.i.z);
}

inline FQuaternion operator-(FQuaternion q1, FQuaternion q2)
{
    return FQuaternion(q1.r - q2.r, q1.i.x - q2.i.x, q1.i.y - q2.i.y, q1.i.z - q2.i.z);
}

inline FQuaternion operator*(FQuaternion q1, FQuaternion q2)
{
    return FQuaternion(q1.r * q2.r - q1.i.x * q2.i.x
        - q1.i.y * q2.i.y - q1.i.z * q2.i.z,
        q1.r * q2.i.x + q1.i.x * q2.r
        + q1.i.y * q2.i.z - q1.i.z * q2.i.y,
        q1.r * q2.i.y + q1.i.y * q2.r
        + q1.i.z * q2.i.x - q1.i.x * q2.i.z,
        q1.r * q2.i.z + q1.i.z * q2.r
        + q1.i.x * q2.i.y - q1.i.y * q2.i.x);
}

inline FQuaternion operator*(FQuaternion q, float s)
{
    return FQuaternion(q.r * s, q.i.x * s, q.i.y * s, q.i.z * s);
}

inline FQuaternion operator*(float s, FQuaternion q)
{
    return FQuaternion(q.r * s, q.i.x * s, q.i.y * s, q.i.z * s);
}

inline FQuaternion operator*(FQuaternion q, FVector v)
{
    return FQuaternion(-(q.i.x * v.x + q.i.y * v.y + q.i.z * v.z),
        q.r * v.x + q.i.y * v.z - q.i.z * v.y,
        q.r * v.y + q.i.z * v.x - q.i.x * v.z,
        q.r * v.z + q.i.x * v.y - q.i.y * v.x);
}

inline FQuaternion operator*(FVector v, FQuaternion q)
{
    return FQuaternion(-(q.i.x * v.x + q.i.y * v.y + q.i.z * v.z),
        q.r * v.x + q.i.z * v.y - q.i.y * v.z,
        q.r * v.y + q.i.x * v.z - q.i.z * v.x,
        q.r * v.z + q.i.y * v.x - q.i.x * v.y);
}

inline FQuaternion operator/(FQuaternion q, float s)
{
    return FQuaternion(q.r / s, q.i.x / s, q.i.y / s, q.i.z / s);
}

inline float getAngle(FQuaternion q)
{
    return (float)(2 * acos(q.r));
}

inline FVector getAxis(FQuaternion q)
{
    FVector v;
    float m;

    v = q.getVector();
    m = magnitude(v);

    if (m <= 0.0001f)
        return FVector();
    else
        return v / m;
}

inline FQuaternion rotateQuat(FQuaternion q1, FQuaternion q2)
{
    return q1 * q2 * (~q1);
}

inline FVector rotateVector(FQuaternion q, FVector v)
{
    FQuaternion t;
    t = q * v * (~q);
    return t.getVector();
}

inline float degToRad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

inline float radToDeg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

inline FQuaternion makeQFromEulerAngles(float x, float y, float z)
{
    FQuaternion     q;
    double     roll = degToRad(x);
    double     pitch = degToRad(y);
    double     yaw = degToRad(z);

    double     cyaw, cpitch, croll, syaw, spitch, sroll;
    double     cyawcpitch, syawspitch, cyawspitch, syawcpitch;

    cyaw = cos(0.5f * yaw);
    cpitch = cos(0.5f * pitch);
    croll = cos(0.5f * roll);
    syaw = sin(0.5f * yaw);
    spitch = sin(0.5f * pitch);
    sroll = sin(0.5f * roll);

    cyawcpitch = cyaw * cpitch;
    syawspitch = syaw * spitch;
    cyawspitch = cyaw * spitch;
    syawcpitch = syaw * cpitch;

    q.r = (float)(cyawcpitch * croll + syawspitch * sroll);
    q.i.x = (float)(cyawcpitch * sroll - syawspitch * croll);
    q.i.y = (float)(cyawspitch * croll + syawcpitch * sroll);
    q.i.z = (float)(syawcpitch * croll - cyawspitch * sroll);

    return q;
}

inline FVector makeEulerAnglesFromQ(FQuaternion q)
{
    double     r11, r21, r31, r32, r33, r12, r13;
    double     q00, q11, q22, q33;
    double     tmp;
    FVector     u;

    q00 = q.r * q.r;
    q11 = q.i.x * q.i.x;
    q22 = q.i.y * q.i.y;
    q33 = q.i.z * q.i.z;

    r11 = q00 + q11 - q22 - q33;
    r21 = 2 * (q.i.x * q.i.y + q.r * q.i.z);
    r31 = 2 * (q.i.x * q.i.z - q.r * q.i.y);
    r32 = 2 * (q.i.y * q.i.z + q.r * q.i.x);
    r33 = q00 - q11 - q22 + q33;

    tmp = fabs(r31);
    if (tmp > 0.999999)
    {
        r12 = 2 * (q.i.x * q.i.y - q.r * q.i.z);
        r13 = 2 * (q.i.x * q.i.z + q.r * q.i.y);

        u.x = radToDeg(0.0f); // roll
        u.y = radToDeg((float)(-(M_PI / 2) * r31 / tmp));   // pitch
        u.z = radToDeg((float)atan2(-r12, -r31 * r13)); // yaw
        return u;
    }

    u.x = radToDeg((float)atan2(r32, r33)); // roll
    u.y = radToDeg((float)asin(-r31));      // pitch
    u.z = radToDeg((float)atan2(r21, r11)); // yaw
    return u;
}