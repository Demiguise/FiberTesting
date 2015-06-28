#ifndef ENVECTORS_H
#define ENVECTORS_H

class EnMatrix3x3;
class EnMatrix4x4;

class EnVector2
{
public:
	EnVector2();
	EnVector2(const float& initX, const float& initY);
	~EnVector2();
	static EnVector2 Zero();
	EnVector2 Normalized();
	float GetMagnitude() const;
	EnVector2& operator+= (const EnVector2& rhs);
	EnVector2& operator-= (const EnVector2& rhs);
	
	float x;
	float y;
};
EnVector2 operator+ (EnVector2 lhs, const EnVector2& rhs);
EnVector2 operator- (EnVector2 lhs, const EnVector2& rhs);

class EnVector3
{
public:
	EnVector3();
	EnVector3(const float& initX, const float& initY, const float& initZ);
	~EnVector3();
	static EnVector3 Zero();
	float ADot(const EnVector3& rhs) const; //Algebraic Dot product - Returns scalar value
	float FindAngleBetween(const EnVector3& rhs) const;
	EnVector3 Cross(const EnVector3& rhs);
	EnVector3 MatrixMult3x3(const EnMatrix3x3& rhs) const;
	EnVector3 MatrixMult4x4(const EnMatrix4x4& rhs) const;
	EnVector3 Normalized();
	float GetMagnitude() const;
	float GetSqMagnitude() const;
	EnVector3& operator+= (const EnVector3& rhs);
	EnVector3& operator-= (const EnVector3& rhs);
	float& operator[] (int index);

	float x;
	float y;
	float z;
};
EnVector3 operator+ (EnVector3 lhs, const EnVector3& rhs);
EnVector3 operator- (EnVector3 lhs, const EnVector3& rhs);
EnVector3 operator* (EnVector3 lhs, const float& rhs);

class EnVector4
{
public:
	EnVector4();
	EnVector4(const float& initX, const float& initY, const float& initZ, const float& initW);
	~EnVector4();
	static EnVector4 Zero();
	EnVector4 Normalized();
	float GetMagnitude() const;

	EnVector4& operator+= (const EnVector4& rhs);
	EnVector4& operator-= (const EnVector4& rhs);
	float& operator[] (int index);
	float x;
	float y;
	float z;
	float w;
};
EnVector4 operator+ (EnVector4 lhs, const EnVector4& rhs);
EnVector4 operator- (EnVector4 lhs, const EnVector4& rhs);
EnVector4 operator* (EnVector4 lhs, const float& rhs);

namespace Util
{
	EnVector2 ScalarProduct2D(const EnVector2& v, const float& s);
	EnVector2 ScalarDivision2D(const EnVector2& v, const float& s);
	EnVector3 ScalarProduct3D(const EnVector3& v, const float& s);
	EnVector3 ScalarDivision3D(const EnVector3& v, const float& s);
}

#endif //~ENVECTORS_H
