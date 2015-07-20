#include "stdafx.h"
#include "Camera.h"

CCamera::CCamera()
{
	m_position = EnVector3::Zero();
	m_rotation = EnVector3::Zero();
	m_scale = EnVector3::Zero();
	m_localToWorld = EnMatrix4x4::Identity();
}

CCamera::~CCamera()
{
}