#include "stdafx.h"
#include <wtypes.h>
#include <cmath>
#include <vector>
#include "EnMatrix.h"
#include "EnVectors.h"

//======== Engine Matrix2x2
//	In the Form:
//	| A B |
//	| C D |
//	Where each column is a 2D Vector. Eg (A,C) & (B,D)
EnMatrix2x2::EnMatrix2x2()
{
	//Init into Identity Matrix;
	c[0] = EnVector2(1.0f, 0.0f);
	c[1] = EnVector2(0.0f, 1.0f);
}

EnMatrix2x2::EnMatrix2x2(const EnVector2& c1, const EnVector2& c2)
{
	c[0] = c1;
	c[1] = c2;
}

EnMatrix2x2::~EnMatrix2x2()
{
}

EnMatrix2x2 EnMatrix2x2::Identity()
{
	return EnMatrix2x2(	EnVector2(1.0f, 0.0f),
						EnVector2(0.0f, 1.0f));
}

float EnMatrix2x2::GetDeterminant()
{
	return	(c[0].x * c[1].y) -
			(c[0].y * c[1].x);
}

//	Engine Matrix3x3
//	In the Form:
//	| A B C |
//	| D E F |
//	| G H I |
//	Where each column is a 3D Vector. Eg (A,D,G), (B,E,H), (C,F,I)
EnMatrix3x3::EnMatrix3x3()
{
	//Init into Identity Matrix;
	c[0] = EnVector3(1.0f, 0.0f, 0.0f);
	c[1] = EnVector3(0.0f, 1.0f, 0.0f);
	c[2] = EnVector3(0.0f, 0.0f, 1.0f);
}

EnMatrix3x3::EnMatrix3x3(const EnVector3& c1, const EnVector3& c2, const EnVector3& c3)
{
	c[0] = c1;
	c[1] = c2;
	c[2] = c3;
}

EnMatrix3x3::~EnMatrix3x3()
{
}

EnMatrix3x3 EnMatrix3x3::Identity()
{
	return EnMatrix3x3(	EnVector3(1.0f, 0.0f, 0.0f),
						EnVector3(0.0f, 1.0f, 0.0f),
						EnVector3(0.0f, 0.0f, 1.0f));
}

// Will NOT overwrite the matrix if returns false. 
bool EnMatrix3x3::Invert() 
{
	EnMatrix3x3 cofactorMatrix;
	EnMatrix2x2 minorMatrix;
	float determinant = GetDeterminant();
	if (determinant == 0) { return false; }
	else
	{
		//Create new matrix containing determinants of each minor matrix.
		for (UINT i = 0 ; i < 3 ; ++i)
		{
			for (UINT j = 0 ; j < 3 ; ++j)
			{
				minorMatrix = CreateMinor(i, j);
				// +2 here as I'm working from a zero index. Can't really be doing 0+0 as a power.
				cofactorMatrix.c[i][j] = pow(-1.f, (i + j + 2.f)) * minorMatrix.GetDeterminant();
			}
		}
		//Now to flip the Matrix along the left->right diagonal.
		cofactorMatrix.Transpose();
	};
	//Finally, multiply the cofactor matrix by the reciprocal of the determinant
	//to get the inverse matrix!
	for (UINT i = 0 ; i < 3 ; ++i)
	{
		c[i] = Util::ScalarProduct3D(cofactorMatrix.c[i], 1/determinant);
	}
	return true;
}

void EnMatrix3x3::Transpose()
{
	Util::SwapValues(c[1][0], c[0][1]);
	Util::SwapValues(c[2][0], c[0][2]);
	Util::SwapValues(c[2][1], c[1][2]);
}

EnMatrix2x2 EnMatrix3x3::CreateMinor(const int& row, const int& column)
{
	std::vector<float> tempVector;
	for ( UINT i = 0 ; i < 3 ; ++i)
	{
		if (i != row)
		{
			for ( UINT j = 0 ; j < 3 ; ++j)
			{
				if (j != column)
				{
					tempVector.push_back(c[i][j]);
				}
			}
		}
	}
	return EnMatrix2x2(	EnVector2(tempVector[0], tempVector[1]),
						EnVector2(tempVector[2], tempVector[3]));
}

float EnMatrix3x3::GetDeterminant()
{
	return	(c[0].x * c[1].y * c[2].z) +	//AEI
			(c[0].y * c[1].z * c[2].x) +	//BFG
			(c[0].z * c[1].x * c[2].y) -	//CDH
			(c[0].x * c[1].z * c[2].y) -	//AFH
			(c[0].y * c[1].x * c[2].z) -	//BFI
			(c[0].z * c[1].y * c[2].x);		//CEG
}

//======== Engine Matrix4x4
//	In the Form:
//	| A B C D |
//	| E F G H |
//	| I J K L |
//	| M N O P |
//	Where each column is a 4D vector. Eg (A,E,I,M), (B,F,J,N)... etc
EnMatrix4x4::EnMatrix4x4()
{
	//Init into Identity Matrix;
	c[0] = EnVector4(1.0f, 0.0f, 0.0f, 0.0f);
	c[1] = EnVector4(0.0f, 1.0f, 0.0f, 0.0f);
	c[2] = EnVector4(0.0f, 0.0f, 1.0f, 0.0f);
	c[3] = EnVector4(0.0f, 0.0f, 0.0f, 1.0f);
	
}

EnMatrix4x4::EnMatrix4x4(const EnVector4& c1, const EnVector4& c2, const EnVector4& c3, const EnVector4& c4)
{
	c[0] = c1;
	c[1] = c2;
	c[2] = c3;
	c[3] = c4;
}

EnMatrix4x4::~EnMatrix4x4()
{
}

EnMatrix4x4 EnMatrix4x4::Identity()
{
	return EnMatrix4x4(	EnVector4(1.0f, 0.0f, 0.0f, 0.0f),
						EnVector4(0.0f, 1.0f, 0.0f, 0.0f),
						EnVector4(0.0f, 0.0f, 1.0f, 0.0f),
						EnVector4(0.0f, 0.0f, 0.0f, 1.0f));
}

//This uses the idea that the inverse of a rotational matrix is its transpose.
//If the matrix is not free from shear and scale this will inevitably fail to 
//give the correct results. Use caution. I have yet to implement an inverse function
//for a 4x4 matrix as it appears slightly more difficult than 3x3 at a cursory glance.
EnVector3 EnMatrix4x4::RotationalInverse(const EnVector3& v)
{
	EnVector3 tempVector = v;
	EnMatrix3x3 tempRotation = EnMatrix3x3( EnVector3(c[0].x, c[0].y, c[0].z),
											EnVector3(c[0].x, c[0].y, c[0].z),
											EnVector3(c[0].x, c[0].y, c[0].z));
	tempRotation.Transpose();
	tempVector.x -= c[3].x;
	tempVector.y -= c[3].y;
	tempVector.z -= c[3].z;
	return EnVector3 ( tempVector.MatrixMult3x3(tempRotation));
}

void EnMatrix4x4::Transpose()
{
	Util::SwapValues(c[1][0], c[0][1]);
	Util::SwapValues(c[2][0], c[0][2]);
	Util::SwapValues(c[3][0], c[0][3]);
	Util::SwapValues(c[2][1], c[1][2]);
	Util::SwapValues(c[3][1], c[1][3]);
	Util::SwapValues(c[3][2], c[2][3]);
}

namespace Util
{
	template<class T> void SwapValues(T& a, T& b)
	{
		T c = a;
		a = b;
		b = c;
	}

	EnMatrix3x3 ScalarProduct3x3(const EnMatrix3x3& m, const float& s)
	{
		EnMatrix3x3 newMatrix;
		for(int i = 0 ; i < 3 ; ++i)
		{
			newMatrix.c[i].x = m.c[i].x * s;
			newMatrix.c[i].y = m.c[i].y * s;
			newMatrix.c[i].z = m.c[i].z * s;
		}
		return newMatrix;
	}
};