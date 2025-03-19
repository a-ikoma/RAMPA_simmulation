#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include "LinkAnt.h"
#include <random>
#include "AllocateData.h"
#include <future>
#include "ModelPartition.h"

class ResourceAnt {
private:
	std::map<int, bool> embResults;
	std::map<int, LinkAnt*> bestAnts;
	std::map<int, double> bestTime;


	double feromonRate;

	int antSedai;
	int antNum;
	double feromonParam;
	double costParam;
	AllocateData* allocData;


	std::map<DDCGraph::Map::vertex_descriptor, double> resourceCosts;
	int linkThreadParam;
	int searchItr;
	std::map<int, std::map<int, double>> feromonInfo;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>> settingFeromons;
	double baseFeromon;

	void setFeromon(DDCGraph* ddcGraph, Request* req, bool sucessFlg, double tmpCost);
	bool checkRange(int node, DDCGraph* d, std::vector<int>& resRange);
	void threadAnt2(std::map<int, int> rMap, int num, DDCGraph* d, Request* req, double feroP, double costP, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCost);
	double calcAllCost(DDCGraph* ddcGraph, Request* req, std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> EdgeMap);

	
	std::vector
		< std::vector
		<std::vector
		< std::pair<std::pair<double, double>,
		std::vector<std::pair<std::pair<DDCGraph::Map::edge_descriptor, int>, int >
		>>>>> appEdges;
	std::vector<double> otherAppLimits;



	std::vector<std::map<int, std::vector<std::vector<std::pair<int, int>>>>> linktasks;
	std::vector<double> limits;

	int hopLimit;
	
public:
	ResourceAnt();
	ResourceAnt(DDCGraph* ddcGraph, int sedai, int antnum, double feroP, double costP, double feroRate, int hopLim, 
		std::vector<Request*>* reqsData, std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemapsData, std::vector<std::map<int, int>*>* resourcemapsData,
		double bestCostData);
	bool ResourceEmbedding(DDCGraph* ddcGraph, Request* req);

	LinkAnt* bestLinkAnt;
	std::map<int, int> resourceMap;
	std::map<int, int> residualData;
	double allCost;

	std::vector<int> selectedNodes;

	double bestCost;



	int antColonySedai;


	std::vector<Request*>* reqs;

	std::vector<std::map<int, int>*>* resourcemaps;
	std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps;



	bool maxResSearchFlg;

};

