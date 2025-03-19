#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include "LinkAnt.h"
#include <random>
#include "AllocateData.h"
#include <future>
#include "ModelPartition.h"

class ResourceAnt {//リソースの埋め込み場所を探索するアリ
private:
	//std::mutex mtx_;
	std::map<int, bool> embResults;
	std::map<int, LinkAnt*> bestAnts;
	std::map<int, double> bestTime;


	double feromonRate;

	int antSedai;//蟻の世代数
	int antNum;//１世代の蟻の数
	double feromonParam;//フェロモンの重要度パラメータ
	double costParam;//コストの重要度パラメータ
	AllocateData* allocData;


	//std::map<DDCGraph::Map::edge_descriptor, double> edgeCosts;
	std::map<DDCGraph::Map::vertex_descriptor, double> resourceCosts;
	int linkThreadParam;
	int searchItr;
	std::map<int, std::map<int, double>> feromonInfo;//フェロモンの情報(エッジ番号、(コア番号、フェロモン))
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>> settingFeromons;
	double baseFeromon;

	void setFeromon(DDCGraph* ddcGraph, Request* req, bool sucessFlg, double tmpCost);
	bool checkRange(int node, DDCGraph* d, std::vector<int>& resRange);
	void threadAnt2(std::map<int, int> rMap, int num, DDCGraph* d, Request* req, double feroP, double costP, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCost);
	double calcAllCost(DDCGraph* ddcGraph, Request* req, std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> EdgeMap);//提案手法の時間を重要視して探索する場合のコスト計算関数

	
	std::vector //各アプリ
		< std::vector //各プロセス列
		<std::vector//各プロセス
		< std::pair<std::pair<double, double>,//ページフォールト数,演算リソースとかの時間
		std::vector<std::pair<std::pair<DDCGraph::Map::edge_descriptor, int>, int >
		>>>>> appEdges;
	std::vector<double> otherAppLimits;//ページフォールト数と制限時間



	std::vector<std::map<int, std::vector<std::vector<std::pair<int, int>>>>> linktasks;//アプリごとのタスクのリンク
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
	std::map<int, int> residualData;//リソースの残余数(ノード番号,残余領域)
	double allCost;//割り当て総コスト

	std::vector<int> selectedNodes;

	double bestCost;



	int antColonySedai;//資源探索の世代番号


	std::vector<Request*>* reqs;

	std::vector<std::map<int, int>*>* resourcemaps;
	std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps;//アプリごとのエッジマップ



	bool maxResSearchFlg;//解が見つからないときに、特定のGPUだけで資源を決定するかのフラグ

};

