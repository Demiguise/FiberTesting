#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common/EnVectors.h"
#include "Common/EnMatrix.h"
#include "Common/Quaternion.h"

class CCamera
{
public:
	CCamera();
	~CCamera();

	EnVector3 m_position;
	EnVector3 m_rotation;
	EnVector3 m_scale;
	EnMatrix4x4 m_localToWorld;
	Quaternion m_quaternion;
};

#endif //~__CAMERA_H__
