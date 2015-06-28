#include "stdafx.h"
#include <cmath>
#include <assert.h>
#include "EnVectors.h"
#include "EnMatrix.h"

//======== Engine Vector2 (Aka, 2 dimensional vector)
EnVector2::EnVector2()
	: x(0), y(0){}

EnVector2::EnVector2(const float& initX, const float& initY)
	: x(initX), y(initY){}

EnVector2::~EnVector2()
{
}

EnVector2 EnVector2::Zero()
{
	return EnVector2(0.0f, 0.0f);
}

EnVector2 EnVector2::Normalized()
{
	float mag = GetMagnitude();
	if (mag == 0) { return EnVector2(); }
	return EnVector2((x/mag), (y/mag));
}

float EnVector2::GetMagnitude() const
{
	float a = x * x;
	float b = y * x;
	return sqrt(a+b);
}

//Member operators
EnVector2& EnVector2::operator+= (const EnVector2& rhs)
{
	x = x + rhs.x;
	y = y + rhs.y;
	return *this;
}

EnVector2& EnVector2::operator-= (const EnVector2& rhs)
{
	x = x - rhs.x;
	y = y - rhs.y;
	return *this;
}

//Non-Member operators
EnVector2 operator+ (EnVector2 lhs, const EnVector2& rhs)
{
	lhs += rhs;
	return lhs;
}

EnVector2 operator- (EnVector2 lhs, const EnVector2& rhs)
{
	lhs -= rhs;
	return lhs;
}

//======== Engine Vector3 (Aka, 3 dimensional vector)
static const EnVector3 Zero = EnVector3(0,0,0);

EnVector3::EnVector3()
	: x(0), y(0), z(0){}

EnVector3::EnVector3(const float& initX, const float& initY, const float& initZ)
	: x(initX), y(initY), z(initZ){}

EnVector3::~EnVector3()
{
}


EnVector3 EnVector3::Zero()
{
	return EnVector3(0.0f, 0.0f, 0.0f);
}

EnVector3 EnVector3::Cross(const EnVector3& rhs)
{
	float lhsMag = GetMagnitude();
	float rhsMag = rhs.GetMagnitude();
	float sineValue = sin(FindAngleBetween(rhs));
	EnVector3 n = EnVector3((y*rhs.z) - (z*rhs.y),
							(z*rhs.x) - (x*rhs.z),
							(x*rhs.y) - (y*rhs.x));
	return Util::ScalarProduct3D(n.Normalized(), (lhsMag*rhsMag*sineValue));
}

//Returns Radians (Via the _CMATH_::acos function)
float EnVector3::FindAngleBetween(const EnVector3& rhs) const
{
	float lhsMag = GetMagnitude();
	float rhsMag = rhs.GetMagnitude();
	if ((lhsMag == 0) || (rhsMag == 0)) { return 0; }
	float dotProduct = ADot(rhs);
	return acos(dotProduct/(lhsMag * rhsMag));
}

//Algebraic Dot product - Returns scalar value
float EnVector3::ADot(const EnVector3& rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}


EnVector3 EnVector3::MatrixMult3x3(const EnMatrix3x3& rhs) const
{
	return EnVector3(	(rhs.c[0].x * x) + (rhs.c[1].x * y) + (rhs.c[2].x * z),
						(rhs.c[0].y * x) + (rhs.c[1].y * y) + (rhs.c[2].y * z),
						(rhs.c[0].z * x) + (rhs.c[1].z * y) + (rhs.c[2].z * z));
}

EnVector3 EnVector3::MatrixMult4x4(const EnMatrix4x4& rhs) const
{
	return EnVector3(	(rhs.c[0].x * x) + (rhs.c[1].x * y) + (rhs.c[2].x * z) + rhs.c[3].x,
						(rhs.c[0].y * x) + (rhs.c[1].y * y) + (rhs.c[2].y * z) + rhs.c[3].y,
						(rhs.c[0].z * x) + (rhs.c[1].z * y) + (rhs.c[2].z * z) + rhs.c[3].z);
}


EnVector3 EnVector3::Normalized()
{
	float mag = GetMagnitude();
	if (mag == 0) { return EnVector3(); }
	return EnVector3((x/mag), (y/mag), (z/mag));
}

float EnVector3::GetMagnitude() const
{
	return sqrt(GetSqMagnitude());
}

float EnVector3::GetSqMagnitude() const
{
	return (x*x) + (y*y) + (z*z);
}

//Member operators

//Unsure if I like this being used on a vector.
//Great for matrix manipulation though.
float& EnVector3::operator[] (int index)
{
	assert ((index <= 2) && (index >= 0));
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		return x;
	}
}

EnVector3& EnVector3::operator+= (const EnVector3& rhs)
{
	x = x + rhs.x;
	y = y + rhs.y;
	z = z + rhs.z;
	return *this;
}

EnVector3& EnVector3::operator-= (const EnVector3& rhs)
{
	x = x - rhs.x;
	y = y - rhs.y;
	z = z - rhs.z;
	return *this;
}

//Non-member operators
EnVector3 operator- (EnVector3 lhs, const EnVector3& rhs)
{
	lhs -= rhs;
	return lhs;
}

EnVector3 operator+ (EnVector3 lhs, const EnVector3& rhs)
{
	lhs += rhs;
	return lhs;
}

EnVector3 operator* (EnVector3 lhs, const float& rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	return lhs;
}

//======== Engine Vector4 (Aka, 4 dimensional vector)
EnVector4::EnVector4()
	: x(0), y(0), z(0), w(0) {}

EnVector4::EnVector4(const float& initX, const float& initY, const float& initZ, const float& initW)
{
	x = initX;
	y = initY;
	z = initZ;
	w = initW;
}

EnVector4::~EnVector4()
{
}

EnVector4 EnVector4::Zero()
{
	return EnVector4(0.0f, 0.0f, 0.0f, 0.0f);
}

EnVector4 EnVector4::Normalized()
{
	float mag = GetMagnitude();
	if (mag == 0) { return EnVector4(); }
	return EnVector4((x/mag), (y/mag), (z/mag), (w/mag));
}

float EnVector4::GetMagnitude() const
{
	float a = x * x;
	float b = y * y;
	float c = z * z;
	float d = w * w;
	return sqrt(a+b+c+d);
}

//Member operators

float& EnVector4::operator[] (int index)
{
	assert ((index <= 3) && (index >= 0));
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		return x;
	}
}

EnVector4& EnVector4::operator+= (const EnVector4& rhs)
{
	x = x + rhs.x;
	y = y + rhs.y;
	z = z + rhs.z;
	w = w + rhs.w;
	return *this;
}

EnVector4& EnVector4::operator-= (const EnVector4& rhs)
{
	x = x - rhs.x;
	y = y - rhs.y;
	z = z - rhs.z;
	w = w - rhs.w;
	return *this;
}

//Non-member operators
EnVector4 operator- (EnVector4 lhs, const EnVector4& rhs)
{
	lhs -= rhs;
	return lhs;
}

EnVector4 operator+ (EnVector4 lhs, const EnVector4& rhs)
{
	lhs += rhs;
	return lhs;
}

EnVector4 operator* (EnVector4 lhs, const float& rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	lhs.w *= rhs;
	return lhs;
}


namespace Util
{

	EnVector2 ScalarProduct2D(const EnVector2& v, const float& s)
	{
		float x = v.x * s;
		float y = v.y * s;
		return EnVector2(x, y);
	}

	EnVector2 ScalarDivision2D(const EnVector2& v, const float& s)
	{
		float x = v.x / s;
		float y = v.y / s;
		return EnVector2(x, y);
	}

	EnVector3 ScalarProduct3D(const EnVector3& v, const float& s)
	{
		float x = v.x * s;
		float y = v.y * s;
		float z = v.z * s;
		return EnVector3(x, y, z);
	}

	EnVector3 ScalarDivision3D(const EnVector3& v, const float& s)
	{
		float x = v.x / s;
		float y = v.y / s;
		float z = v.z / s;
		return EnVector3(x, y, z);
	}
};