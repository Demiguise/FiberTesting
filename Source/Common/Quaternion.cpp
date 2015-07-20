#include "stdafx.h"
#include <cmath>
#include "Quaternion.h"
#include "EnMatrix.h"

//======== Quaternion (For Rotations)
// A Quaternion is made up of four parts, 1 scalar and a 3D vector.
// The vector is often written as (i, j, k) to denote the imaginary numbers that it uses.
// However, to get the usefulness of the already present 3D vectors, I'll call them (x, y, z).
// That is, unless someone can think of a reason not to...

Quaternion::Quaternion()
	: scalar(1), vector(EnVector3::Zero()) {}

Quaternion::Quaternion(const float& angle, const EnVector3& axis)
{
	float halfAngle = DEG2RAD(angle/2.0f);
	float sineAngle = sin(halfAngle);
	scalar = cos(halfAngle);
	vector = EnVector3 ((axis.x * sineAngle), (axis.y * sineAngle), (axis.z * sineAngle));
}

Quaternion::Quaternion(const float& initS, const float& initX, const float& initY, const float& initZ)
	: scalar(initS), vector(initX, initY, initZ) {}

Quaternion::~Quaternion() {}

Quaternion Quaternion::Normalized()
{
	float magnitude = GetMagnitude();
	if (magnitude == 0)
	{
		return Quaternion();
	}
	return Quaternion((scalar/magnitude), (vector.x/magnitude), (vector.y/magnitude), (vector.z/magnitude));
}

float Quaternion::GetMagnitude()
{
	float a = scalar * scalar;
	float b = vector.x * vector.x;
	float c = vector.y * vector.y;
	float d = vector.z * vector.z;
	return sqrt(a+b+c+d);
}

EnMatrix3x3 Quaternion::To3x3Matrix()
{
	//There's no nice way of laying this out, so I'm doing it in the most readable way.
	EnMatrix3x3 rotationMatrix;
	//First Row
	rotationMatrix.c[0].x = 1 - (2*(vector.y*vector.y) + 2*(vector.z*vector.z));
	rotationMatrix.c[1].x = 2*vector.x*vector.y - 2*vector.z*scalar;
	rotationMatrix.c[2].x = 2*vector.x*vector.z + 2*vector.y*scalar;
	//Second Row
	rotationMatrix.c[0].y = 2*vector.x*vector.y + 2*vector.z*scalar;
	rotationMatrix.c[1].y = 1 - (2*(vector.x*vector.x) + 2*(vector.z*vector.z));
	rotationMatrix.c[2].y = 2*vector.y*vector.z - 2*vector.x*scalar;
	//Third Row
	rotationMatrix.c[0].z = 2*vector.x*vector.z - 2*vector.y*scalar;
	rotationMatrix.c[1].z = 2*vector.y*vector.z + 2*vector.x*scalar;
	rotationMatrix.c[2].z = 1 - (2*(vector.x*vector.x) + 2*(vector.y*vector.y));
	return rotationMatrix;
}

//Member operators
Quaternion& Quaternion::operator*= (const Quaternion& rhs)
{
	float tempScalar =  scalar * rhs.scalar - vector.ADot(rhs.vector);
	vector = EnVector3(	Util::ScalarProduct3D(rhs.vector, scalar) +
						Util::ScalarProduct3D(vector, rhs.scalar) +
						vector.Cross(rhs.vector));
	scalar = tempScalar;
	return *this;
}


//Non-Member operators
Quaternion operator* (Quaternion lhs, const Quaternion& rhs)
{
	lhs *= rhs;
	return lhs;
}