#ifndef PERF_DB_H
#define PERF_DB_H

#include <map>
#include <string>
#include <vector>

class PerformanceDB
{
public:
	PerformanceDB();
	~PerformanceDB();

	void AddData(std::string name, float data);
	float GetAvgForData(std::string name);
	void ReportAvgs();

private:
	void MergeTemp();
	typedef std::map<std::string, std::vector<float>> TPerfMap;
	typedef std::pair<std::string, std::vector<float>> TPerfPair;
	TPerfMap m_perfMap;
	TPerfMap m_tempMap;
};
extern PerformanceDB* g_pPerfDB;

#endif //~PERF_DB_H
