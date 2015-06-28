#include "stdafx.h"
#include "PerfDB.h"
#include <mutex>

PerformanceDB* g_pPerfDB = NULL;
std::mutex dbLock;

PerformanceDB::PerformanceDB()
{
}

PerformanceDB::~PerformanceDB()
{
}

void PerformanceDB::AddData(std::string name, float data)
{
	dbLock.lock();
	m_tempMap[name].push_back(data);
	dbLock.unlock();
}

float PerformanceDB::GetAvgForData(std::string name)
{
	return 0.f;
}

void PerformanceDB::MergeTemp()
{
	TPerfMap::iterator it = m_tempMap.begin();
	for (; it!= m_tempMap.end() ; ++it)
	{
		TPerfMap::iterator itPerm = m_perfMap.find(it->first);
		if (itPerm == m_perfMap.end())
		{
			m_perfMap[it->first] = it->second;
		}
		else
		{
			std::vector<float>& times = it->second;
			itPerm->second.insert(itPerm->second.end(), times.begin(), times.end());
		}
		it->second.clear();
	}
}

void PerformanceDB::ReportAvgs()
{
	dbLock.lock();
	TPerfMap::iterator it = m_tempMap.begin();
	for (; it != m_tempMap.end() ; ++it)
	{
		std::vector<float>& times = it->second;
		float highest = 0.f;
		float lowest = 1000.f;
		float avg = 0.f;
		int numJobs = times.size();
		for (int i = 0 ; i < numJobs ; ++i)
		{
			float& time = times[i];
			avg += time;
			if (time > highest)
			{
				highest = time;
			}
			else if (time < lowest)
			{
				lowest = time;
			}
		}
		avg /= numJobs;

		DebugLog("	[%s] N: %d | AvgPN:%f | (%f...%f) | Total:%f", it->first.c_str(), numJobs, avg, lowest, highest, (numJobs * avg));
	}
	MergeTemp();
	dbLock.unlock();
}