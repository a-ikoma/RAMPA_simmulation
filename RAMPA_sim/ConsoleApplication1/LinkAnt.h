#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include <map>
#include <random>


class LinkAnt{//リンクの埋め込み場所を探索するアリ
private:


	//仮想リンクと物理エッジとそのコアの組をマッピング
	bool missFlg = false;//調査失敗かどうか
	
	double feromonParam;//フェロモンの重要度パラメータ
	double costParam;//コストの重要度パラメータ




	//比較手法かどうか（予測時間を短くする）
	void initCandInfo(DDCGraph* ddcGraph, Request* req);
	void embIntLink(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor, int coreNum,int source,int target, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateCandInfo(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum, int source, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateRate(DDCGraph* ddcGraph, Request* req);
	double baseFeromon;//候補として選ばれていないリンクのフェロモンの値
	//std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts;
	std::vector<int> distance;
	std::vector<int> tarDistance;
	double procMinTime;//各プロセスの最小時間を保存
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> tmpRate;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> checkDelay;//各リンクの遅延データ
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
	std::map<int, std::vector<std::pair<std::pair<int,int>, int>>> EdgeMap;//<リンクの隣接ノードの組とコア番号の組>
	double allCost;
	//std::map<int, std::map<int,double>>* feromonInfo;//コア情報の参照
	std::map<DDCGraph::Map::edge_descriptor, std::map<int,bool>>* nallocCandinfo;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* allocCandinfo;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> linkant_newAllocatedLink;//リンクアントで新しく割り当てられたやつ
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> newAllocCandinfo;//新しく割り当てられたやつの集合

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* enableinfo;
	//リンク番号とコア番号とソースノードで（レートと遅延）->map[リンク番号][map<>]
	std::map<int, std::map<int, std::map<int, double[2]>>> rate_delay;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int,double>>> tmpDelay;//比較手法用の遅延保持マップ
	//void updateNewGraph();
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>>* tmp_lightPaths;//探索段階における仮の光パス

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> newlightPaths;//探索段階における新規の光パス

	double feromonValue(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,
		DDCGraph::Map::edge_descriptor e, int coreNum);

	bool checkOcupy(DDCGraph* d, Request* req, DDCGraph::Map::edge_descriptor e, int core);

	void checkAllocApp(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core);


	int hopLimit;//ホップ制限
	double predictTime;//比較手法用の時間

	std::map < DDCGraph::Map::edge_descriptor, std::map < int, std::pair<std::map<int, double>, int>>> newAppRate;//このアプリの埋め込みで行為sンされるレート量を保存

	std::map< DDCGraph::Map::edge_descriptor, std::map < int,std::vector<std::pair<std::map<int,double>,int>>>> useAppRates;

	std::vector //各アプリ
	< std::vector //各プロセス列
	<std::vector//各プロセス
	< std::pair<std::pair<double, double>,//ページフォールト数,演算リソースとかの時間
	std::vector<std::pair< std::pair<DDCGraph::Map::edge_descriptor, int>, int >
	>>>>>* appEdges;
	std::vector<double>* otherAppLimits;//ページフォールト数と制限時間


	std::vector<std::map<int, std::vector<std::vector<std::pair<int, int>>>>> linktasks;//アプリごとのタスクのリンク
	std::vector<double> limits;
	double baseCost;//枝切りのため


	std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps;//アプリごとのエッジマップ
	std::vector<Request*>* reqs;
	std::vector<std::map<int, int>*>* resourcemaps;

	std::set<int> appIDs;
	double maxTraffic[4];

	double allocatedResourceCost;


	DDCGraph::Map::edge_descriptor curLightPathSourceEdge;//現在突入している光パスの最初に入るリンク
	int curLightPathSourceCore;//現在突入している光パスの最初に入るリンクのコア番号

	std::map<int, int> resourceMap;
	double curLightPathCost;//現在の光パスのコスト



	std::map<int, std::map<int, double>> resourcePairPropagation;//ソースノードからターゲットノードまでの伝搬遅延＋スイッチ処理遅延の総和
	double currentResourcePairPropagation;//今探索中の資源間の伝搬遅延

	std::pair<std::vector<int>, std::pair<double, double>> partitionSol;//モデル分割方法の解

};