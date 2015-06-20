#ifndef ENMATRIX_H
#define ENMATRIX_H

#include "EnVectors.h"

class EnMatrix2x2
{
public:
	EnMatrix2x2();
	EnMatrix2x2(const EnVector2& c1, const EnVector2& c2);
	~EnMatrix2x2();

	static EnMatrix2x2 Identity();
	float GetDeterminant();

	EnVector2 c[2];
};

class EnMatrix3x3
{
public:
	EnMatrix3x3();
	EnMatrix3x3(const EnVector3& c1, const EnVector3& c2, const EnVector3& c3);
	~EnMatrix3x3();

	static EnMatrix3x3 Identity();
	EnMatrix2x2 CreateMinor(const int& row, const int& column);
	bool Invert();
	void Transpose();
	float GetDeterminant();

	EnVector3 c[3];
};

class EnMatrix4x4
{
public:
	EnMatrix4x4();
	EnMatrix4x4(const EnVector4& c1,const EnVector4& c2,const EnVector4& c3,const EnVector4& c4);
	~EnMatrix4x4();

	static EnMatrix4x4 Identity();
	EnVector3 RotationalInverse(const EnVector3& v);
	void Transpose();

	EnVector4 c[4];
};

namespace Util
{
	template<class T> void SwapValues(T& a, T& b);
	EnMatrix3x3 ScalarProduct3x3(const EnMatrix3x3&m, const float& s);
}

#endif //~ENMATRIX_H
