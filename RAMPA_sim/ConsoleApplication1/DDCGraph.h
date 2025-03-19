#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <stdio.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>
#include <queue>
#include <boost/graph/depth_first_search.hpp>
#include <omp.h>


class DDCGraph
{

public:

    struct coreInfo {
        bool enable;
        bool candFlg;
        int coreNum;
        std::map<int, double> rate;
        double feromon;
        std::set<int> intCores;
        std::vector<std::pair<std::map<int, double>,int>> useAppRate;
        std::map<int, double> delay;
    };
    // variable
private:
    std::string file;


    struct node {
        int number;
        int resource;
        int block;
        int residual;
        int id;
        double trans_delay;
        double cost;
        double flops;
        double feromon;
        double capacity;
        double memBand;
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double propagation;
        double cost;
        std::unordered_map<int,coreInfo> core;
        std::vector<int> adjNode;
        int passWeight;
        int coreNumber;
    };

    struct bundle {
        std::string name;
    };

    
    
    // accessor
public:
    int allocatePolicy;
    double decreaseRate;
    typedef boost::adjacency_list<//multisetS
        boost::listS, boost::vecS, boost::undirectedS,
        node,
        link,
        bundle
    > Map;
    DDCGraph(std::string filename,double gensui, double initFeromon,int allocPolicy);
    DDCGraph();

    DDCGraph::Map createGraph();

    double bandwidth;

    double initialFeromon;

    std::map<int,std::vector<int>> distances;

    int CPUResidual;
    int GPUResidual;
    int memResidual;

    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;
private:

    std::vector<std::pair<int, int>> gpuPairs;
    

    std::vector<Map::vertex_descriptor> cpuIter;
    std::vector<Map::vertex_descriptor> gpuIter;
    std::vector<Map::vertex_descriptor> memIter;
    std::vector<Map::vertex_descriptor> packetIter;
    typedef Map::vertex_descriptor   Vertex;
    void derive_shortest_path(Map::vertex_descriptor s, int edge1, int edge2,
        std::map<int, long long>& dis, std::map<int, long long>& dis2,
        std::map<int, int>& prev,
        std::map<int, int>& count, std::map<int, int>& edgeCount);


    std::map<Map::edge_descriptor,std::map<int,std::map<int, long long>>> shortestHop;//shortest hop between nodes
    std::map < Map::edge_descriptor, std::map<int, std::map<int, long long>>> shortestHopExceptEdge;//shortest hop between nodes (except path through target edge)
    std::map < Map::edge_descriptor, std::map<int, std::map<int, int>>> shortestPaths;//number of shortest paths between nodes
    std::map < Map::edge_descriptor, std::map<int, std::map<int, int>>> shortestPathsExceptEdge;//number of shortest paths between nodes(except path through target edge)

public:
    Map graph;
    double maxGPUCost;//GPUコストんお最大値
    int allocCount = 0;
    std::map<Map::edge_descriptor, std::vector<int>> initCandInfoMap;
    std::map<Map::edge_descriptor, std::vector<int>> initEnableInfoMap;

    void decreaseNodeFeromon();
    void resetFeromon();//フェロモン値のリセット


    void setResidual(std::map<int, int> residualData);
    int calcCoreMapID(std::set<int> cores);
    void updateResourceCost(std::map<Map::vertex_descriptor,double>* resourceCosts, std::map<int, int>* residualData);
    void updateLinkCost(std::map<Map::edge_descriptor, double>* edgeCosts, std::map<Map::vertex_descriptor, double>* resourceCosts,std::map<int, int>* resData);

    std::vector<int> allocateResourceRange;
    void setAllocateResourceRange();
    void eraseLightPath();
    void setAllocCandInfo();
    double propagationValue;
    double propagationValue_resource;
    std::string graphName;


    void deriveNodeDistance(Map::vertex_descriptor s, std::vector<int>& dis);

    double circuitDelay;

    std::vector<Map::edge_descriptor> swLinks;

    int memAllResidual;
    int CPUAllResidual;
    int GPUAllResidual;


    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> nallocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> allocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> enableinfo;


    std::vector<int> fixAllocateCandidates;



    int allNodeNum;


    std::vector<int> maxResidualNode;

};