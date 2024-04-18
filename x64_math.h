#pragma once

#include <immintrin.h>

union xmm {
    f32 Float;
    v2 Vector2;
    v3 Vector3;
    v4 Vector4;
    __m128 Register;

    constexpr xmm() : Register() { Register = _mm_xor_ps(Register, Register); }
    constexpr xmm(f32 Value) : Float(Value) { };
    constexpr xmm(v2 Value) : Vector2(Value) { };
    constexpr xmm(v3 Value) : Vector3(Value) { };
    constexpr xmm(const v4 &Value) : Vector4(Value) { };
    constexpr xmm(const __m128 &XMM) : Register(XMM) { };

    explicit operator f32() const { return Float; }
    explicit operator v2() const { return Vector2; }
    explicit operator v3() const { return Vector3; }
    explicit operator v4() const { return Vector4; }
    operator __m128() const { return Register; }

    MATHCALL xmm CreateMask(bool Value) {
        xmm Zero = xmm();
        xmm Result = (Value) ? xmm(_mm_cmpeq_ps(Zero, Zero)) : Zero;
        return Result;
    }
};

MATHCALL f32 Sqrt(f32 Value) {
    xmm Result = _mm_sqrt_ss(xmm(Value));
    return (f32)Result;
}
MATHCALL f32 Max(f32 A, f32 B) {
    xmm Result = _mm_max_ss(xmm(A), xmm(B));
    return (f32)Result;
}
MATHCALL f32 Min(f32 A, f32 B) {
    xmm Result = _mm_min_ss(xmm(A), xmm(B));
    return (f32)Result;
}
MATHCALL f32 Negate(f32 Value) {
    xmm SignBit = xmm(F32SignBit);
    xmm Result = _mm_xor_ps(SignBit, xmm(Value));
    return (f32)Result;
}
MATHCALL f32 Sign(f32 Value) {
    xmm Result = 1.0f;
    Result = _mm_or_ps(Result, xmm(F32SignBit));
    return (f32)Result;
}
MATHCALL f32 Reciprocal(f32 Value) {
    xmm Result = _mm_rcp_ss(xmm(Value));
    return (f32)Result;
}
MATHCALL f32 Saturate(f32 Value) {
    if (Value < 0.0f) return 0.0f;
    if (Value > 1.0f) return 1.0f;
    return Value;
}
MATHCALL f32 FMA(f32 A, f32 B, f32 C) {
    return 0.0f;
}

constexpr inline v2::v2(f32 X) {
    xmm xmm0 = _mm_broadcastss_ps(xmm(X));
    *this = (v2)xmm0;
}
constexpr inline v3::v3(f32 X) {
    xmm xmm0 = _mm_broadcastss_ps(xmm(X));
    *this = (v3)xmm0;
}
constexpr inline v4::v4(f32 X) {
    xmm xmm0 = _mm_broadcastss_ps(xmm(X));
    *this = (v4)xmm0;
}
constexpr inline v2::v2(f32 X, f32 Y) {
    xmm xmm0 = _mm_set_ps(0.0f, 0.0f, Y, X);
    *this = (v2)xmm0;
}
constexpr inline v3::v3(f32 X, f32 Y, f32 Z) {
    xmm xmm0 = _mm_set_ps(0.0f, Z, Y, X);
    *this = (v3)xmm0;
}
constexpr inline v4::v4(f32 X, f32 Y, f32 Z, f32 W) {
    xmm xmm0 = _mm_set_ps(W, Z, Y, X);
    *this = (v4)xmm0;
}


MATHCALL u32 PopCount(u32 a) {
	u32 Result = _mm_popcnt_u32(a);
	return Result;
}
MATHCALL u64 PopCount(u64 a) {
	u64 Result = _mm_popcnt_u64(a);
	return Result;
}

MATHCALL u32 RoundUpPowerOf2(u32 Value, u32 Power2) {
    Assert(PopCount(Power2) == 1);
    u32 Result = Value;
    u32 Mask = Power2 - 1;
    Result += Mask;
    Result &= ~Mask;
    return Result;
}

MATHCALL u64 RoundUpPowerOf2(u64 Value, u64 Power2) {
    Assert(PopCount(Power2) == 1);
    u64 Result = Value;
    u64 Mask = Power2 - 1;
    Result += Mask;
    Result &= ~Mask;
    return Result;
}

constexpr inline f32 v2::Dot(const v2 &A, const v2 &B) {
    v2 Mul = A * B;
    return Mul.x + Mul.y;
}
constexpr inline f32 v2::LengthSquared(const v2 &Value) {
    return v2::Dot(Value, Value);
}
constexpr inline f32 v2::Length(const v2 &Value) {
    return Sqrt(v2::LengthSquared(Value));
}
constexpr inline v2 v2::Normalize(const v2 &Value) {
    f32 LengthSquared = v2::LengthSquared(Value);

    bool LengthGreaterThanZero = LengthSquared > F32Epsilon;
    xmm Mask = xmm::CreateMask(LengthGreaterThanZero);

    f32 Length = Sqrt(LengthSquared);
    v2 Result = Value * Reciprocal(Length);

    xmm MaskedResult = _mm_and_ps(xmm(Result), Mask);
    return (v2)MaskedResult;
}
MATHCALL v2 operator+(const v2 &A, const v2 &B) {
    xmm Result = _mm_add_ps(xmm(A), xmm(B));
    return (v2)Result;
}
MATHCALL v2 operator-(const v2 &A, const v2 &B) {
    xmm Result = _mm_sub_ps(xmm(A), xmm(B));
    return (v2)Result;
}
MATHCALL v2 operator*(const v2 &A, const v2 &B) {
    xmm Result = _mm_mul_ps(xmm(A), xmm(B));
    return (v2)Result;
}
MATHCALL v2 operator/(const v2 &A, const v2 &B) {
    xmm Result = _mm_div_ps(xmm(A), xmm(B));
    return (v2)Result;
}

constexpr inline f32 v3::Dot(const v3 &A, const v3 &B) {
    v3 Mul = A * B;
    return Mul.x + Mul.y + Mul.z;
}
constexpr inline f32 v3::LengthSquared(const v3 &Value) {
    return v3::Dot(Value, Value);
}
constexpr inline f32 v3::Length(const v3 &Value) {
    return Sqrt(v3::LengthSquared(Value));
}
constexpr inline v3 v3::Normalize(const v3 &Value) {
    f32 LengthSquared = v3::LengthSquared(Value);

    bool LengthGreaterThanZero = LengthSquared > F32Epsilon;
    xmm Mask = xmm::CreateMask(LengthGreaterThanZero);

    f32 Length = Sqrt(LengthSquared);
    v3 Result = Value * Reciprocal(Length);

    xmm MaskedResult = _mm_and_ps(xmm(Result), Mask);
    return (v3)MaskedResult;
}
constexpr inline v3 v3::Cross(const v3 &A, const v3 &B) {
    v3 Result;
    Result.x = A.y * B.z - A.z * B.y;
    Result.y = A.z * B.x - A.x * B.z;
    Result.z = A.x * B.y - A.y * B.x;
    return Result;
}
MATHCALL v3 operator+(const v3 &A, const v3 &B) {
    xmm Result = _mm_add_ps(xmm(A), xmm(B));
    return (v3)Result;
}
MATHCALL v3 operator-(const v3 &A, const v3 &B) {
    xmm Result = _mm_sub_ps(xmm(A), xmm(B));
    return (v3)Result;
}
MATHCALL v3 operator*(const v3 &A, const v3 &B) {
    xmm Result = _mm_mul_ps(xmm(A), xmm(B));
    return (v3)Result;
}
MATHCALL v3 operator/(const v3 &A, const v3 &B) {
    xmm Result = _mm_div_ps(xmm(A), xmm(B));
    return (v3)Result;
}