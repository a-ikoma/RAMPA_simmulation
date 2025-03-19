#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include "ResourceAnt.h"
#include <future>
#include "AllocateData.h"
//#include <boost/thread.hpp>
class AntColony
{
    
private:
    std::map<int, ResourceAnt> bestAnts;

    int antSedai;
    int antNum;
    double feromonGensui;
    double feromonParam;
    double costParam;
    double exeTimeParam;
    AllocateData* allocData;
    int threads;
    bool hikakuFlg;
    double feromonRate;

public:
    AntColony();
    void threadAnt(int ants, int num, DDCGraph* d, Request* req, double feroP, double costP, int curSedai);
    AntColony(AllocateData* alloc, int sedai, int antnum, double gensui, double feroP, double costP, int threadNumber, double feroRate);
    bool procEmb(DDCGraph* d, Request* req);
    ResourceAnt bestEmb;
    std::map<DDCGraph::Map::edge_descriptor, double> edgeCosts;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> tmpDelay;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> nallocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> allocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> enableinfo;
    std::map< DDCGraph::Map::edge_descriptor, std::map < int, std::vector<std::pair<std::map<int, double>,int>>>> useAppRates;
    std::map<int, std::map<int, std::map<int, double[2]>>> rate_delay;
    std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> EdgeMap;
    std::pair<std::vector<int>, std::pair<double, double>> partitionSol;
    void updateLinkAntInfo();
    double bestCost;
    void updateNewGraph(DDCGraph* d, Request* req);
    std::vector<bool> results;
    std::map<int, int> resourceMap;
    std::map<int, int> residualData;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;
    int relatedAppNum;

    int syuusokuNum;

    double allocationTime;

    std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>> HukanouLightPath;


    std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*> edgemaps;//アプリごとのエッジマップ
    std::vector<Request*> reqs;
    std::vector<std::map<int, int>*> resourcemaps;
    int optStage;
    bool curFoundFlg;


};

