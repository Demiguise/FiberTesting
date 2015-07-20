#ifndef QUATERNION_H
#define QUATERNION_H

#include "EnVectors.h"

class EnMatrix3x3;

class Quaternion
{
public:
	Quaternion();
	Quaternion(const float& angle, const EnVector3& axis);
	Quaternion(const float& initS, const float& initI, const float& initJ, const float& initK);
	~Quaternion();
	Quaternion Normalized();
	EnMatrix3x3 To3x3Matrix();
	float GetMagnitude();
	float scalar;
	EnVector3 vector;
	Quaternion& operator*= (const Quaternion& rhs);
};
Quaternion operator* (Quaternion lhs, const Quaternion& rhs);
#endif //~QUATERNION_H
