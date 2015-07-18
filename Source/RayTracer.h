#ifndef __CRAYTRACER_H_
#define __CRAYTRACER_H_

#include <windows.h>
#include <vector>
#include "Common/EnVectors.h"

struct PixelCoord
{
	PixelCoord ()
		: x(0)
		, y(0) {}
	PixelCoord(const int _x, const int _y)
		:	x(_x)
		,	y(_y) {}
	int x;
	int y;
};

struct AABB
{
	EnVector3	min;
	EnVector3 max;
	bool Overlaps(AABB& rhs)
	{
		if (max.x < rhs.min.x || min.x > rhs.max.x) return false;
		if (max.y < rhs.min.y || min.y > rhs.max.y) return false;
		if (max.z < rhs.min.z || min.z > rhs.max.z) return false;
		return true;
	}
};

struct SSphere
{
	SSphere(EnVector3 _pos, EnVector4 _colour, float _r)
		: pos(_pos)
		, colour(_colour)
		, r(_r)
	{
		aabb.max = EnVector3(pos.x + r, pos.y + r, pos.z + r);
		aabb.min = EnVector3(pos.x - r, pos.y - r, pos.z - r);
	}
	EnVector3 pos;
	EnVector4 colour;
	AABB aabb;
	float r;
};

//Considered Omni white light
struct SLight
{
	EnVector3	pos;
	float str;
	int id;
};

typedef std::vector<SSphere> TSceneGeom;
typedef std::vector<SLight> TSceneLights;

struct SSceneInfo
{
	TSceneGeom geom;
	TSceneLights lights;
};

struct SRayCastInfo
{
	SRayCastInfo()
		: pHitGeom(NULL) {}
	EnVector3 pos;
	EnVector3 normal;
	SSphere* pHitGeom;
};

struct SPixelData
{
	SPixelData()
		:	pos(0,0)
		,	geom(NULL)
		,	finalColour(0.f,0.f,0.f,255.f)
	{
	}
	PixelCoord pos;
	TSceneGeom* geom;
	EnVector4 finalColour;
};

class CRayTracer
{
public:
	CRayTracer();
	~CRayTracer();

	void OutputImage();
	SPixelData* GetPixelDataForPos(const int x, const int y) 
	{ 
		return &m_jobDatas[x][y]; 
	}

private:
	static void __stdcall SeedChunk(LPVOID lpParam);
	static void __stdcall CalculatePixelColour(LPVOID lpParam);
	static SRayCastInfo FindClosestPoint(const EnVector3& pos, const EnVector3& dir, TSceneGeom* pGeom);
	static bool CastRay(const EnVector3& pos, const EnVector3& dir, SSphere* pGeom, SRayCastInfo& info);
	static EnVector4 CalculateLighting(SRayCastInfo& info, SSceneInfo* pScene);

	typedef std::vector<SPixelData> TPixelData;
	SSceneInfo m_scene;
	TPixelData m_jobDatas[gWinWidth];
};

extern CRayTracer* g_pRayTracer;

#endif //~__CRAYTRACER_H_