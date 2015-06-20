#include "stdafx.h"

#include "RayTracer.h"
#include "FiberScheduler.h"
#include "lodepng.h"

#include <vector>

#define NUM_CHUNKS 16

CRayTracer::CRayTracer()
{
	SSphere sphere, sphere2, sphere3;
	sphere.pos = EnVector3(120.f, 150.f, 0.f);
	sphere.colour.x = 255.f;
	sphere.r = 20.f;

	sphere2.pos = EnVector3(350.f, 250.f, 0.f);
	sphere2.colour.y = 255.f;
	sphere2.r = 14.f;

	sphere3.pos = EnVector3(320.f, 100.f, 0.f);
	sphere3.colour.z = 255.f;
	sphere3.r = 35.f;
	m_geom.push_back(sphere);
	m_geom.push_back(sphere2);
	m_geom.push_back(sphere3);
	for (int w = 0 ; w < kWidth ; ++w)
	{
		m_jobDatas[w].resize(kHeight);
	}

	CREATEJOB(chunkJob, SeedChunk);
	m_chunks.resize(NUM_CHUNKS);

	int numColRows = sqrt(NUM_CHUNKS);
	int chunkW = kWidth / numColRows;
	int chunkH = kHeight / numColRows;
	for (int i = 0 ; i < NUM_CHUNKS ; ++i)
	{
		SChunk& newChunk = m_chunks[i];
		newChunk.h = chunkH;
		newChunk.w = chunkW;
		newChunk.y = (i / numColRows) * chunkH;	//Gives row offset
		newChunk.x = (i % numColRows) * chunkW; //Gives column offset
		newChunk.geom = &m_geom;
		newChunk.m_jobDatas = m_jobDatas;
		newChunk.id = i;
		CFiberScheduler::Schedule(chunkJob, eFP_High, NULL, &m_chunks[i]);
	}
}

CRayTracer::~CRayTracer()
{
}

void CRayTracer::SeedChunk(LPVOID lpParam)
{
	SChunk* data = GETJOBINFO(SChunk*);

	CREATEJOB(rayJob, CalculatePixelColour);
	UINT32 numJobs = 0;
	CFiberCounter pixelCounter (data->h * data->w);
	CFiber::Log("Seeding for Chunk %d. (pX:%d | pY:%d)", data->id, data->x, data->y);
	for (int w = 0 ; w < data->w; ++w)
	{
		for (int h = 0 ; h < data->h ; ++h)
		{
			PixelCoord pPos = PixelCoord(w + data->x, h + data->y);
			SPixelJobData& pixelData = data->m_jobDatas[pPos.x][pPos.y];
			pixelData.pos = pPos;
			pixelData.geom = data->geom;
			CFiberScheduler::Schedule(rayJob, eFP_Normal, &pixelCounter, &pixelData);
			++numJobs;
		}
	}
	CFiber::Log("%u Jobs seeded for Chunk %d.", numJobs, data->id);
	CFiber::YieldForCounter(&pixelCounter);
	CFiber::Log("Chunk %d finished.", data->id);
}

void CRayTracer::OutputImage()
{
	std::vector<unsigned char> image;
	image.resize(kWidth * kHeight * 4);

	for (int w = 0 ; w < kWidth ; ++w)
	{
		for (int h = 0 ; h < kHeight ; ++h)
		{
			EnVector4& colour = m_jobDatas[w][h].finalColour;
			image[4 * kWidth * h + 4 * w + 0] = (int)colour.x;
			image[4 * kWidth * h + 4 * w + 1] = (int)colour.y;
			image[4 * kWidth * h + 4 * w + 2] = (int)colour.z;
			image[4 * kWidth * h + 4 * w + 3] = (int)colour.w;

			//Cool fractal code
			//image[4 * kWidth * h + 4 * w + 0] = 255 * !(w & h);
			//image[4 * kWidth * h + 4 * w + 1] = w ^ h;
			//image[4 * kWidth * h + 4 * w + 2] = w | h;
			//image[4 * kWidth * h + 4 * w + 3] = 255;
		}
	}

	unsigned error = lodepng::encode("D:\\test.png", image, kWidth, kHeight);
}

/*static*/ void CRayTracer::CalculatePixelColour(LPVOID lpParam)
{
	SPixelJobData* data = GETJOBINFO(SPixelJobData*);
	SRayCastInfo info;

	TSceneGeom::iterator it = data->geom->begin();
	while (it != data->geom->end())
	{
		//Cast ray from "pixel" position out towards the positive Z axis
		if (CRayTracer::CastRay(EnVector3(data->pos.x, data->pos.y, -1000.f), EnVector3(0.f, 0.f, 1.f), &(*it), info))
		{
			//We hit something, get the overall colour of the obj
			data->finalColour = it->colour;
			data->finalColour.w = 255;
		}
		++it;
	}
	data->finalColour.w = 255;
}

/*static*/ bool CRayTracer::CastRay(const EnVector3 pos, const EnVector3 dir, SSphere* pGeom, SRayCastInfo& info)
{
	EnVector3 lPos2SPos = pGeom->pos - pos;
	float c = lPos2SPos.GetMagnitude();
	float v = dir.ADot(lPos2SPos);
	float d = (pGeom->r * pGeom->r) - (c*c - v*v);

	if (d < 0.f)
	{
		//No hit
		return false;
	}
	else
	{
		//single hit
		info.pos = pos + (dir * (v-sqrt(d)));
		return true;
	}
}