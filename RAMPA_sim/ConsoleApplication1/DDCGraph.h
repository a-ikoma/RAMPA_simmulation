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


class DDCGraph//基盤となるマイクロデータセンターネットワークを作成するクラス
{

public:

    struct coreInfo {
        bool enable;//実行時に利用されるコアかどうか
        bool candFlg;//選択候補になるかどうかのフラグ
        int coreNum;
        std::map<int, double> rate;//パケット到着率
        double feromon;//フェロモン
        std::set<int> intCores;//統合リンクの集合
        std::vector<std::pair<std::map<int, double>,int>> useAppRate;//アプリごとのパケット到着率を記録したやつ(割り当て解除処理の時に利用する)
        std::map<int, double> delay;//応答遅延＋伝播遅延
    };
    // variable
private:
    std::string file;


    struct node {
        int number;
        int resource;//メモリ1,cpu2,GPU3,OCS4,パケットスイッチ5
        int block;//コア数またはメモリのブロックの総数
        int residual;//残余ブロック数
        int id;//リソースの性能ごとのid
        double trans_delay;//ノードの転送処理遅延
        double cost;//コスト
        double flops;//FLOPS
        double feromon;//フェロモン
        double capacity;//VRAM容量
        double memBand;//メモリ帯域幅
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double propagation;//伝播遅延
        double cost;//リンクのコスト
        std::unordered_map<int,coreInfo> core;//ハッシュはご存じの通りで
        std::vector<int> adjNode;
        int passWeight;//最小ホップ探索のためだけの値
        int coreNumber;//ノード間のリンク数
    };

    struct bundle {
        std::string name;
    };

    
    
    // accessor
public:
    int allocatePolicy;
    double decreaseRate;//フェロモンの減衰率
    typedef boost::adjacency_list<//multisetS
        boost::listS, boost::vecS, boost::undirectedS,
        node,    // 頂点のBundleプロパティ
        link, // 辺のBundleプロパティ
        bundle  // グラフのBundleプロパティ
    > Map;
    DDCGraph(std::string filename,double gensui, double initFeromon,int allocPolicy);
    DDCGraph();

    DDCGraph::Map createGraph();



    double bandwidth;


    double initialFeromon;//フェロモンの初期値

    std::map<int,std::vector<int>> distances;



    int CPUResidual;
    int GPUResidual;
    int memResidual;

    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;//キーとしてソースノードの番号、バリューとして、キーが最初に通過するコアの番号である、通過リンクとコア番号のペアの集合
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
    std::map<Map::edge_descriptor, std::vector<int>> initCandInfoMap;//candFlgがtrueのエッジディスクリプターとコアをもつ
    std::map<Map::edge_descriptor, std::vector<int>> initEnableInfoMap;//enableFlgの初期値だけを持つやつ

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

    std::vector<Map::edge_descriptor> swLinks;//スイッチ間のリンクの集合

    int memAllResidual;//初期の残余
    int CPUAllResidual;//初期の残余
    int GPUAllResidual;//初期の残余


    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> nallocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> allocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> enableinfo;


    std::vector<int> fixAllocateCandidates;//同じGPUプール内の資源からすべて選ぶ時の候補



    int allNodeNum;


    std::vector<int> maxResidualNode;

};