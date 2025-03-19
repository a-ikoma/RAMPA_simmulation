#pragma once
#include "Request.h"
#include <map>
#include <vector>
#include "DDCGraph.h"
#include "LinkAnt.h"
class AllocateData{
private:

    struct allocInfo {
        int id;
        Request* req;
        std::map<int, int> resourceMap;
        std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> edgeMap;
        std::pair<std::vector<int>, std::pair<double, double>> partition;
    };


public:
    AllocateData();
    bool calcExeTime(LinkAnt* ant, DDCGraph* ddcGraph);
    void pushAllocData(int allocId,Request* request, std::map<int, int> resourceInfo, std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> edgeInfo, DDCGraph* d, std::pair<std::vector<int>, std::pair<double, double>> partitionData);
    void removeAllocData(int id, DDCGraph* ddcGraph);
    bool disIntTime(int id, DDCGraph* ddcGraph);
    void disIntegratePart(DDCGraph* ddcGraph, int orgCore, int disCore, DDCGraph::Map::edge_descriptor e, int id);
    void resetAll();
    std::map<int, allocInfo> info;




private:
    
    void edgeMapSetting(DDCGraph* ddcGraph,int ignoreID);
    bool checkOcupy(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core);
    int countAppNum(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core);
    int countAppRate(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core, DDCGraph::Map::edge_descriptor e2, int core2);
};

