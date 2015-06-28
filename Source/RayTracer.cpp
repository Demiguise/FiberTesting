#include "stdafx.h"

#include "RayTracer.h"
#include "Fibers/FiberScheduler.h"
#include "lodepng.h"
#include "Debug/ScopedTimer.h"

#include <vector>

#define NUM_CHUNKS 16

extern CRayTracer* g_pRayTracer = 0;

CRayTracer::CRayTracer()
{
	//Define Spheres
	SSphere sphereRed, sphereGreen, sphereBlue, spherePurp, sphereYellow;
	
	sphereRed.pos = EnVector3(100.f, 150.f, 0.f);
	sphereRed.colour.x = 255.f;
	sphereRed.r = 20.f;
	sphereGreen.pos = EnVector3(350.f, 300.f, 0.f);
	sphereGreen.colour.y = 255.f;
	sphereGreen.r = 14.f;
	sphereBlue.pos = EnVector3(350.f, 80.f, 0.f);
	sphereBlue.colour.z = 255.f;
	sphereBlue.r = 35.f;

	//Overlap Test
	spherePurp.pos = EnVector3(230.f, 160.f, 20.f);
	spherePurp.colour.x = 255.f;
	spherePurp.colour.z = 255.f;
	spherePurp.r = 20.f;
	sphereYellow.pos = EnVector3(250.f, 160.f, -20.f);
	sphereYellow.colour.x = 255.f;
	sphereYellow.colour.y = 255.f;
	sphereYellow.r = 20.f;

	m_scene.geom.push_back(sphereRed);
	m_scene.geom.push_back(sphereGreen);
	m_scene.geom.push_back(sphereBlue);
	m_scene.geom.push_back(spherePurp);
	m_scene.geom.push_back(sphereYellow);

	//Define Lights
	SLight lightLeft, lightRight;
	lightLeft.pos = EnVector3(100.f, 75.f, 0.f);
	lightLeft.str = 100000.f;
	lightLeft.id = 0;
	lightRight.pos = EnVector3(400.f, 100.f, 0.f);
	lightRight.str = 100000.f;
	lightRight.id = 1;

	m_scene.lights.push_back(lightLeft);
	m_scene.lights.push_back(lightRight);


	for (int w = 0 ; w < gWinWidth ; ++w)
	{
		m_jobDatas[w].resize(gWinHeight);
	}

	CREATEJOB(chunkJob, SeedChunk);

	int numColRows = sqrt(NUM_CHUNKS);
	int chunkW = gWinWidth / numColRows;
	int chunkH = gWinHeight / numColRows;
	for (int i = 0 ; i < NUM_CHUNKS ; ++i)
	{
		CFiberJobData jobData;
		jobData.AddData(i);
		jobData.AddData(chunkH);
		jobData.AddData(chunkW);
		jobData.AddData((i / numColRows) * chunkH);	//Gives row offset
		jobData.AddData((i % numColRows) * chunkW); //Gives column offset
		jobData.AddData(&m_scene);
		jobData.AddData(&m_jobDatas);
		CFiberScheduler::Schedule(chunkJob, eFP_High, jobData, NULL);
	}
}

CRayTracer::~CRayTracer()
{
}

void CRayTracer::SeedChunk(LPVOID lpParam)
{
	CFiberJobData& data = GETJOBINFO;
	int id = data.GetInt(0);
	int chunkH = data.GetInt(1);
	int chunkW = data.GetInt(2);
	int offsetY = data.GetInt(3);
	int offsetX = data.GetInt(4);
	void* pScene = data.GetPointer(5);
	TPixelData* pPixels = (TPixelData*)data.GetPointer(6);

	CREATEJOB(rayJob, CalculatePixelColour);
	UINT32 numJobs = 0;
	CFiberCounter pixelCounter (chunkH * chunkW);
	for (int w = 0 ; w < chunkW; ++w)
	{
		for (int h = 0 ; h < chunkH ; ++h)
		{
			CFiberJobData pixelJob;
			PixelCoord pPos = PixelCoord(w + offsetX, h + offsetY);
			SPixelData* pPixelData = &(*(pPixels + pPos.x))[pPos.y];
			pPixelData->pos = pPos;
			pixelJob.AddData(pPixelData);
			pixelJob.AddData(pScene);
			CFiberScheduler::Schedule(rayJob, eFP_Normal, pixelJob, &pixelCounter);
			++numJobs;
		}
	}
	CFiber::Log("%u Jobs seeded for Chunk %d. (pX:%d | pY:%d)", numJobs, id, offsetX, offsetY);
	CFiber::YieldForCounter(&pixelCounter);
	CFiber::Log("Chunk %d finished.", id);
}

/*static*/ void CRayTracer::CalculatePixelColour(LPVOID lpParam)
{
	PERFTIMER_FUNC();
	CFiberJobData& data = GETJOBINFO;

	SPixelData* pixelData = (SPixelData*)data.GetPointer(0);
	SSceneInfo* pScene = (SSceneInfo*)data.GetPointer(1);

	//Cast ray from "pixel" position out towards the positive Z axis
	SRayCastInfo info = FindClosestPoint(EnVector3(pixelData->pos.x, pixelData->pos.y, -1000.f), EnVector3(0.f, 0.f, 1.f), &pScene->geom);

	if (info.pHitGeom)
	{
		//EnVector4 colour = CRayTracer::CalculateLighting(info, pScene);
		pixelData->finalColour = info.pHitGeom->colour;
		//CFiber::Log("(%d,%d) (%f, %f, %f)", pixelData->pos.x, pixelData->pos.y, colour.x, colour.y, colour.z);
	}
}

EnVector4 CRayTracer::CalculateLighting(SRayCastInfo& info, SSceneInfo* pScene)
{
	EnVector4 colour;
	TSceneLights::iterator lightIter = pScene->lights.begin();
	TSceneGeom::iterator geomIter;
	while (lightIter != pScene->lights.end())
	{
		geomIter = pScene->geom.begin();
		while (geomIter != pScene->geom.end())
		{
			EnVector3 vecToPoint = lightIter->pos - info.pos;
			SRayCastInfo lightInfo = FindClosestPoint(lightIter->pos, vecToPoint.Normalized(), &pScene->geom);
			if (lightInfo.pHitGeom && (lightInfo.pHitGeom == info.pHitGeom))
			{
				//This light has hit the thing! ILLUMINATE IT
				float distToPoint = vecToPoint.GetMagnitude();
				float strAtPoint = lightIter->str / (4 * 3.14 * (distToPoint * distToPoint));
				colour = (info.pHitGeom->colour * strAtPoint);
				//CFiber::Log("Light [%d] has hit point (%f, %f, %f) with a str of %f | Colour from (%f, %f, %f) -> (%f, %f, %f)", lightIter->id, info.pos.x, info.pos.y, info.pos.z, strAtPoint,
				//	info.pHitGeom->colour.x, info.pHitGeom->colour.y, info.pHitGeom->colour.z, colour.x, colour.y, colour.z);
				colour.w = 255.f;
			}
			++geomIter;
		}
		++lightIter;
	}
	return colour;
}

SRayCastInfo CRayTracer::FindClosestPoint(const EnVector3& pos, const EnVector3& dir, TSceneGeom* pGeom)
{
	PERFTIMER_FUNC();
	SRayCastInfo info;
	float sqDistToNearest = 10000.f;

	TSceneGeom::iterator it = pGeom->begin();
	for (; it != pGeom->end() ; ++it)
	{
		SSphere* pSphere = &(*it);
		if (CRayTracer::CastRay(pos, dir, pSphere, info))
		{
			//We hit something, get the overall colour of the obj
			if ((info.pos - pos).GetSqMagnitude() < sqDistToNearest)
			{
				info.pHitGeom = pSphere;
			}
		}
	}

	return info;
}

/*static*/ bool CRayTracer::CastRay(const EnVector3& pos, const EnVector3& dir, SSphere* pGeom, SRayCastInfo& info)
{
	PERFTIMER_FUNC();
	EnVector3 lPos2SPos = pGeom->pos - pos;
	float v = dir.ADot(lPos2SPos);
	float d = (pGeom->r * pGeom->r) - (lPos2SPos.GetSqMagnitude() - v*v);

	if (d < 0.f)
	{
		//No hit
		return false;
	}
	else if (d > 0.f)
	{
		//two hit
		float dSqrt = sqrt(d);
		float distA = v - dSqrt;
		float distB = v + dSqrt;
		info.pos = pos + (dir * (distA < distB ? distA : distB));
	}
	else
	{
		//one hit
		info.pos = pos + (dir * (v-sqrt(d)));
	}
	return true;
}

void CRayTracer::OutputImage()
{
	std::vector<unsigned char> image;
	image.resize(gWinHeight * gWinWidth * 4);

	for (int w = 0 ; w < gWinWidth ; ++w)
	{
		for (int h = 0 ; h < gWinHeight ; ++h)
		{
			EnVector4& colour = m_jobDatas[w][h].finalColour;
			image[4 * gWinWidth * h + 4 * w + 0] = (int)colour.x;
			image[4 * gWinWidth * h + 4 * w + 1] = (int)colour.y;
			image[4 * gWinWidth * h + 4 * w + 2] = (int)colour.z;
			image[4 * gWinWidth * h + 4 * w + 3] = (int)colour.w;

			//Cool fractal code
			//image[4 * kWidth * h + 4 * w + 0] = 255 * !(w & h);
			//image[4 * kWidth * h + 4 * w + 1] = w ^ h;
			//image[4 * kWidth * h + 4 * w + 2] = w | h;
			//image[4 * kWidth * h + 4 * w + 3] = 255;
		}
	}

	unsigned error = lodepng::encode("D:\\ThreadFiberTest\\Renders\\output.png", image, gWinWidth, gWinHeight);
}

void CRayTracer::GetGLPixelOutput(GLfloat* pOutput)
{
	for (int w = 0 ; w < gWinWidth ; ++w)
	{
		for (int h = 0 ; h < gWinHeight ; ++h)
		{
			EnVector4& colour = m_jobDatas[w][h].finalColour;
			*pOutput++ = colour.x;
			*pOutput++ = colour.y;
			*pOutput++ = colour.z;
			*pOutput++ = colour.w;
		}
	}
}