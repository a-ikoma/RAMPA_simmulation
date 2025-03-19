#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include <map>
#include <random>


class LinkAnt{
private:

	bool missFlg = false;
	
	double feromonParam;
	double costParam;

	void initCandInfo(DDCGraph* ddcGraph, Request* req);
	void embIntLink(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor, int coreNum,int source,int target, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateCandInfo(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum, int source, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateRate(DDCGraph* ddcGraph, Request* req);
	double baseFeromon;
	std::vector<int> distance;
	std::vector<int> tarDistance;
	double procMinTime;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> tmpRate;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> checkDelay;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<int>>> coreList;
public:
	LinkAnt();

	LinkAnt(DDCGraph* graph, Request* request, std::map<int, int> map, int limit, double feroP, double costP, double baseFero,
		std::vector<Request*>* reqs, std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps, std::vector<std::map<int, int>*>* resourcemaps, double baseCost, double allocatedResourceCost);
	bool LinkEmbedding(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,DDCGraph* ddcGraph, Request* req, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts);
	void selectLink(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,DDCGraph* ddcGraph, Request* req, 
		int source, int target, ReqGraph::ReqMap::edge_descriptor vedge, std::vector<int>& passNode, 
		std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts,int hops, bool inLghtPath, std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> curLightPath, int lightPathHop);
	void searchCand(DDCGraph* ddcGraph, Request* req, int source,
		std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>& nallocCand, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>& allocCand,
		ReqGraph::ReqMap::edge_descriptor vedge, std::vector<int> passNode, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts,int hops, bool inLghtPath, int curTarget);
	std::map<int, std::vector<std::pair<std::pair<int,int>, int>>> EdgeMap;
	double allCost;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int,bool>>* nallocCandinfo;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* allocCandinfo;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> linkant_newAllocatedLink;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> newAllocCandinfo;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* enableinfo;
	std::map<int, std::map<int, std::map<int, double[2]>>> rate_delay;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int,double>>> tmpDelay;
	//void updateNewGraph();
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>>* tmp_lightPaths;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> newlightPaths;

	double feromonValue(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,
		DDCGraph::Map::edge_descriptor e, int coreNum);

	bool checkOcupy(DDCGraph* d, Request* req, DDCGraph::Map::edge_descriptor e, int core);

	void checkAllocApp(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core);


	int hopLimit;
	double predictTime;

	std::map < DDCGraph::Map::edge_descriptor, std::map < int, std::pair<std::map<int, double>, int>>> newAppRate;

	std::map< DDCGraph::Map::edge_descriptor, std::map < int,std::vector<std::pair<std::map<int,double>,int>>>> useAppRates;

	std::vector
		< std::vector
		<std::vector
		< std::pair<std::pair<double, double>,
		std::vector<std::pair< std::pair<DDCGraph::Map::edge_descriptor, int>, int >
		>>>>>* appEdges;
	std::vector<double>* otherAppLimits;


	std::vector<std::map<int, std::vector<std::vector<std::pair<int, int>>>>> linktasks;
	std::vector<double> limits;
	double baseCost;


	std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps;
	std::vector<Request*>* reqs;
	std::vector<std::map<int, int>*>* resourcemaps;

	std::set<int> appIDs;
	double maxTraffic[4];

	double allocatedResourceCost;


	DDCGraph::Map::edge_descriptor curLightPathSourceEdge;
	int curLightPathSourceCore;

	std::map<int, int> resourceMap;
	double curLightPathCost;

	std::map<int, std::map<int, double>> resourcePairPropagation;
	double currentResourcePairPropagation;

	std::pair<std::vector<int>, std::pair<double, double>> partitionSol;

};