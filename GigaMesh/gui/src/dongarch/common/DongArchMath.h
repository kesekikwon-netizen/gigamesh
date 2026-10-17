/* * DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_MATH_H
#define DONGARCH_MATH_H

#include <cmath>
#include <numbers>
#include "DongArchTypes.h"

//! \file DongArchMath.h
//! \brief C++20 constexpr 수학 함수
//!
//! DongArch3D v4.0의 핵심 수학 라이브러리
//! - constexpr 함수: 컴파일 타임 계산
//! - C++20 std::numbers 사용
//! - Phase 1 (Align) 회전 행렬 계산에 사용

namespace DongArch {
namespace Math {

// ============================================================================
// 3D Vector (간단한 구조체)
// ============================================================================

//! \struct Vec3
//! \brief 3D 벡터 (경량, constexpr 지원)
struct Vec3 {
    double x, y, z;

    //! 생성자
    constexpr Vec3(double x_ = 0.0, double y_ = 0.0, double z_ = 0.0)
        : x(x_), y(y_), z(z_) {}

    //! 벡터 덧셈
    constexpr Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    //! 벡터 뺄셈
    constexpr Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    //! 스칼라 곱셈
    constexpr Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    //! 스칼라 나눗셈
    constexpr Vec3 operator/(double scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }
};

// ============================================================================
// constexpr 수학 함수
// ============================================================================

//! \brief 내적 (Dot Product)
//!
//! \param a 첫 번째 벡터
//! \param b 두 번째 벡터
//! \return a · b
//!
//! constexpr이므로 컴파일 타임에 계산 가능:
//! \code
//! constexpr Vec3 v1{1, 0, 0};
//! constexpr Vec3 v2{0, 1, 0};
//! constexpr double dot = dotProduct(v1, v2); // 0.0 (컴파일 타임)
//! \endcode
constexpr double dotProduct(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

//! \brief 외적 (Cross Product)
//!
//! \param a 첫 번째 벡터
//! \param b 두 번째 벡터
//! \return a × b
//!
//! 오른손 법칙 (Right-hand rule) 적용
constexpr Vec3 crossProduct(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

//! \brief 벡터 길이의 제곱
//!
//! \param v 벡터
//! \return |v|²
//!
//! sqrt 연산을 피하기 위해 사용 (성능 최적화)
constexpr double lengthSquared(const Vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

//! \brief 벡터 길이
//!
//! \param v 벡터
//! \return |v|
//!
//! C++20에서는 constexpr이 아님 (std::sqrt는 비 constexpr)
//! 런타임에만 사용
inline double length(const Vec3& v) {
    return std::sqrt(lengthSquared(v));
}

//! \brief 벡터 정규화 (Normalize)
//!
//! \param v 벡터
//! \return v / |v|
//!
//! 단위 벡터 반환
inline Vec3 normalize(const Vec3& v) {
    double len = length(v);
    if (len < 1e-10) {
        return Vec3(0, 0, 0);  // Zero vector
    }
    return v / len;
}

//! \brief 두 벡터 사이의 각도 (라디안)
//!
//! \param a 첫 번째 벡터
//! \param b 두 번째 벡터
//! \return 각도 (0 ~ π)
inline double angleBetween(const Vec3& a, const Vec3& b) {
    double dot = dotProduct(a, b);
    double lenA = length(a);
    double lenB = length(b);

    if (lenA < 1e-10 || lenB < 1e-10) {
        return 0.0;
    }

    double cosTheta = dot / (lenA * lenB);
    // Clamp to [-1, 1] to avoid acos domain error
    cosTheta = std::max(-1.0, std::min(1.0, cosTheta));

    return std::acos(cosTheta);
}

//! \brief 각도를 라디안으로 변환
//!
//! \param degrees 각도
//! \return 라디안
//!
//! C++20 std::numbers::pi 사용
constexpr double degreesToRadians(double degrees) {
    return degrees * std::numbers::pi / 180.0;
}

//! \brief 라디안을 각도로 변환
//!
//! \param radians 라디안
//! \return 각도
constexpr double radiansToDegrees(double radians) {
    return radians * 180.0 / std::numbers::pi;
}

// ============================================================================
// 회전 행렬 (Rotation Matrix) - Phase 1 Align에서 사용
// ============================================================================

//! \struct Mat3
//! \brief 3x3 행렬 (회전 행렬용)
struct Mat3 {
    double m[3][3];

    //! 단위 행렬
    constexpr Mat3()
        : m{{1, 0, 0},
            {0, 1, 0},
            {0, 0, 1}} {}

    //! 행렬-벡터 곱셈
    constexpr Vec3 operator*(const Vec3& v) const {
        return Vec3(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }

    //! 행렬-행렬 곱셈
    constexpr Mat3 operator*(const Mat3& other) const {
        Mat3 result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i][j] = 0;
                for (int k = 0; k < 3; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
};

//! \brief X축 회전 행렬 생성
//!
//! \param radians 회전 각도 (라디안)
//! \return 3x3 회전 행렬
//!
//! 런타임 함수 (sin, cos는 비 constexpr)
inline Mat3 rotationMatrixX(double radians) {
    Mat3 rot;
    double c = std::cos(radians);
    double s = std::sin(radians);

    rot.m[0][0] = 1;  rot.m[0][1] = 0;  rot.m[0][2] = 0;
    rot.m[1][0] = 0;  rot.m[1][1] = c;  rot.m[1][2] = -s;
    rot.m[2][0] = 0;  rot.m[2][1] = s;  rot.m[2][2] = c;

    return rot;
}

//! \brief Y축 회전 행렬 생성
//!
//! \param radians 회전 각도 (라디안)
//! \return 3x3 회전 행렬
inline Mat3 rotationMatrixY(double radians) {
    Mat3 rot;
    double c = std::cos(radians);
    double s = std::sin(radians);

    rot.m[0][0] = c;   rot.m[0][1] = 0;  rot.m[0][2] = s;
    rot.m[1][0] = 0;   rot.m[1][1] = 1;  rot.m[1][2] = 0;
    rot.m[2][0] = -s;  rot.m[2][1] = 0;  rot.m[2][2] = c;

    return rot;
}

//! \brief Z축 회전 행렬 생성
//!
//! \param radians 회전 각도 (라디안)
//! \return 3x3 회전 행렬
inline Mat3 rotationMatrixZ(double radians) {
    Mat3 rot;
    double c = std::cos(radians);
    double s = std::sin(radians);

    rot.m[0][0] = c;  rot.m[0][1] = -s;  rot.m[0][2] = 0;
    rot.m[1][0] = s;  rot.m[1][1] = c;   rot.m[1][2] = 0;
    rot.m[2][0] = 0;  rot.m[2][1] = 0;   rot.m[2][2] = 1;

    return rot;
}

//! \brief 임의의 축에 대한 회전 행렬 (Rodrigues' rotation formula)
//!
//! \param axis 회전축 (단위 벡터)
//! \param radians 회전 각도 (라디안)
//! \return 3x3 회전 행렬
inline Mat3 rotationMatrixAxis(const Vec3& axis, double radians) {
    Vec3 k = normalize(axis);
    double c = std::cos(radians);
    double s = std::sin(radians);
    double t = 1.0 - c;

    Mat3 rot;
    rot.m[0][0] = c + k.x * k.x * t;
    rot.m[0][1] = k.x * k.y * t - k.z * s;
    rot.m[0][2] = k.x * k.z * t + k.y * s;

    rot.m[1][0] = k.y * k.x * t + k.z * s;
    rot.m[1][1] = c + k.y * k.y * t;
    rot.m[1][2] = k.y * k.z * t - k.x * s;

    rot.m[2][0] = k.z * k.x * t - k.y * s;
    rot.m[2][1] = k.z * k.y * t + k.x * s;
    rot.m[2][2] = c + k.z * k.z * t;

    return rot;
}

// ============================================================================
// 유틸리티 함수
// ============================================================================

//! \brief 두 값 사이 선형 보간 (Linear Interpolation)
//!
//! \param a 시작 값
//! \param b 끝 값
//! \param t 보간 계수 (0.0 ~ 1.0)
//! \return 보간 결과
template<FloatingPoint T>
constexpr T lerp(T a, T b, T t) {
    return a + (b - a) * t;
}

//! \brief 값을 범위로 제한 (Clamp)
//!
//! \param value 값
//! \param min 최소값
//! \param max 최대값
//! \return 제한된 값
template<typename T>
constexpr T clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

} // namespace Math
} // namespace DongArch

#endif // DONGARCH_MATH_H
