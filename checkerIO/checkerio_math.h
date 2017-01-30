#if !defined(CHECKERIO_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define MAX(A, B) ((((A) > (B))) ? (A) : (B)) 
#define MIN(A, B) ((((A) < (B))) ? (A) : (B)) 
#define ABS(A) (MAX((A), ((-1)*(A))))

union v2
{
    struct
    {
        r32 x, y;
    };
    r32 E[2];
};

union v2i
{
    struct
    {
        i32 x, y;
    };
    i32 E[2];
};

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 r, g, b;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 yz;
    };
    r32 E[3];
};

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };
        
        r32 w;        
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };
        
        r32 a;        
    };
    r32 E[4];
};

struct rectangle2
{
    v2 Min;
    v2 Max;
};

struct rectangle3
{
    v3 Min;
    v3 Max;
};

/*
inline v2
V2i(int32 X, int32 Y)
{
    v2 Result = {(r32)X, (r32)Y};
    
    return(Result);
}

inline v2
V2i(uint32 X, uint32 Y)
{
    v2 Result = {(r32)X, (r32)Y};
    
    return(Result);
}
*/

inline v2i
V2i(i32 X, i32 Y)
{
    v2i Result = {X, Y};
    return(Result);
}

inline v2
V2(r32 X, r32 Y)
{
    v2 Result;
    
    Result.x = X;
    Result.y = Y;
    
    return(Result);
}

inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return(Result);
}

inline v3
V3(v2 XY, r32 Z)
{
    v3 Result;
    
    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;
    
    return(Result);
}

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    
    return(Result);
}

inline v3
ToV3(v2 XY, r32 Z)
{
    v3 Result;
    
    Result.xy = XY;
    Result.z = Z;
    
    return(Result);
}

inline v4
ToV4(v3 XYZ, r32 W)
{
    v4 Result;
    
    Result.xyz = XYZ;
    Result.w = W;
    
    return(Result);
}

//
// NOTE(casey): Scalar operations
//
/*
// NOTE(maciek): This can take values ranging from 0 to 1024
inline i32
Lerp(i32 A, i32 t, i32 B)
{
    i32 Result = (A*(1024 - t) + B*t) >> 10;
    return(Result);
}

inline i32
Lerp(i32 A, i32 t, i32 B)
{
    i32 Result = A + (B - A)*t;
    return(Result);
}
*/
inline i32
Lerp(i32 A, i32 t, i32 B)
{
    i32 Inc = A > B ? -1 : 1;
    i32 Result = A + t*Inc;
    return(Result);
}

inline r32
Square(r32 A)
{
    r32 Result = A*A;
    
    return(Result);
}

inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

inline r32
Clamp(r32 Min, r32 Value, r32 Max)
{
    r32 Result = Value;
    
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }
    
    return(Result);
}

inline r32
Clamp01(r32 Value)
{
    r32 Result = Clamp(0.0f, Value, 1.0f);
    
    return(Result);
}

inline r32
SafeRatioN(r32 Numerator, r32 Divisor, r32 N)
{
    r32 Result = N;
    
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }
    
    return(Result);
}

inline r32
SafeRatio0(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
    
    return(Result);
}

inline r32
SafeRatio1(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    
    return(Result);
}

//
// NOTE(casey): v2 operations
//

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}

inline v2
operator*(r32 A, v2 B)
{
    v2 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2
operator*(v2 B, r32 A)
{
    v2 Result = A*B;
    
    return(Result);
}

inline v2 &
operator*=(v2 &B, r32 A)
{
    B = A * B;
    
    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2i
operator*(i32 A, v2i B)
{
    v2i Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2i
operator*(v2i B, i32 A)
{
    v2i Result = A*B;
    
    return(Result);
}

inline v2i &
operator*=(v2i &B, i32 A)
{
    B = A * B;
    
    return(B);
}

inline v2i
operator-(v2i A)
{
    v2i Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2i
operator+(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2i &
operator+=(v2i &A, v2i B)
{
    A = A + B;
    
    return(A);
}

inline v2i
operator-(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline b32
operator==(v2i A, v2i B)
{
    b32 Result = false;
    
    if(A.x == B.x && A.y == B.y)
    {
        Result = true;
    }
    
    return(Result);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};
    
    return(Result);
}

inline r32
Inner(v2 A, v2 B)
{
    r32 Result = A.x*B.x + A.y*B.y;
    
    return(Result);
}

inline r32
LengthSq(v2 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32
Length(v2 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    
    return(Result);
}

//
// NOTE(casey): v3 operations
//

inline v3
operator*(r32 A, v3 B)
{
    v3 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    
    return(Result);
}

inline v3
operator*(v3 B, r32 A)
{
    v3 Result = A*B;
    
    return(Result);
}

inline v3 &
operator*=(v3 &B, r32 A)
{
    B = A * B;
    
    return(B);
}

inline v3
operator-(v3 A)
{
    v3 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return(Result);
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;
    
    return(A);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}

inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    
    return(Result);
}

inline r32
Inner(v3 A, v3 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return(Result);
}

inline r32
LengthSq(v3 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32
Length(v3 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v3
Normalize(v3 A)
{
    v3 Result = A * (1.0f / Length(A));
    
    return(Result);
}

inline v3
Clamp01(v3 Value)
{
    v3 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    
    return(Result);
}

inline v3
Lerp(v3 A, r32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

//
// NOTE(casey): v4 operations
//

inline v4
operator*(r32 A, v4 B)
{
    v4 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;
    
    return(Result);
}

inline v4
operator*(v4 B, r32 A)
{
    v4 Result = A*B;
    
    return(Result);
}

inline v4 &
operator*=(v4 &B, r32 A)
{
    B = A * B;
    
    return(B);
}

inline v4
operator-(v4 A)
{
    v4 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;
    
    return(Result);
}

inline v4
operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

inline v4 &
operator+=(v4 &A, v4 B)
{
    A = A + B;
    
    return(A);
}

inline v4
operator-(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return(Result);
}

inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};
    
    return(Result);
}

inline r32
Inner(v4 A, v4 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    
    return(Result);
}

inline r32
LengthSq(v4 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32
Length(v4 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v4
Clamp01(v4 Value)
{
    v4 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);
    
    return(Result);
}

inline v4
Lerp(v4 A, r32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

//
// NOTE(casey): Rectangle2
//

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    
    return(Result);
}

inline b32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y));
    
    return(Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    
    return(Result);
}

//
// NOTE(casey): Rectangle3
//

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;
    
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;
    
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;
    
    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    
    return(Result);
}

inline b32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.z >= Rectangle.Min.z) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y) &&
                     (Test.z < Rectangle.Max.z));
    
    return(Result);
}

inline b32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) ||
                      (B.Min.x >= A.Max.x) ||
                      (B.Max.y <= A.Min.y) ||
                      (B.Min.y >= A.Max.y) ||
                      (B.Max.z <= A.Min.z) ||
                      (B.Min.z >= A.Max.z));
    return(Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);
    
    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;
    
    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;
    
    return(Result);
}

#define CHECKERIO_MATH_H
#endif