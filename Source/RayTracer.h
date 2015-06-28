#ifndef __CRAYTRACER_H_
#define __CRAYTRACER_H_

#include <windows.h>
#include <vector>
#include "Common/EnVectors.h"

#include "glfw3.h"

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

struct SSphere
{
	EnVector3 pos;
	EnVector4 colour;
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
	void GetGLPixelOutput(GLfloat* pOutput);
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