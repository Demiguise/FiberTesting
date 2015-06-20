#ifndef __CRAYTRACER_H_
#define __CRAYTRACER_H_

#include <windows.h>
#include <vector>
#include "EnVectors.h"

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

typedef std::vector<SSphere> TSceneGeom;

struct SRayCastInfo
{
	EnVector3 pos;
	EnVector3 normal;
};

struct SPixelJobData
{
	PixelCoord pos;
	TSceneGeom* geom;
	EnVector4 finalColour;
};

struct SChunk
{
	int w;
	int h;
	int x;
	int y;
	int id;
	TSceneGeom* geom;
	std::vector<SPixelJobData>* m_jobDatas;
};

class CRayTracer
{
public:
	CRayTracer();
	~CRayTracer();

	void OutputImage();

	static void __stdcall SeedChunk(LPVOID lpParam);
	static void __stdcall CalculatePixelColour(LPVOID lpParam);
	static bool CastRay(const EnVector3 pos, const EnVector3 dir, SSphere* pGeom, SRayCastInfo& info);

private:
	static const int kWidth = 480;
	static const int kHeight = 320;
	TSceneGeom m_geom;
	std::vector<SPixelJobData> m_jobDatas[kWidth];
	std::vector<SChunk> m_chunks;
};

#endif //~__CRAYTRACER_H_