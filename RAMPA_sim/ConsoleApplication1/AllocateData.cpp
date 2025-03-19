#include "AllocateData.h"
using namespace std;


AllocateData::AllocateData()
{
	return;
}


void AllocateData::resetAll() {//全要素の解放
	for (auto &a : info) {
		delete a.second.req;
		a.second.req = nullptr;
	}
}

void AllocateData::pushAllocData(int allocId,Request* request, std::map<int, int> resourceInfo, std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> edgeInfo,DDCGraph* d, std::pair<std::vector<int>, std::pair<double, double>> partitionData) {//割り当て情報をセット
	//エッジマップの更新

	for (const auto& item : info) {
		auto edge_range = edges(info[item.first].req->graph);
		for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {//更新
			for (int j = 0; j < info[item.first].edgeMap[info[item.first].req->graph[*first].number].size(); j++) {
				auto e = info[item.first].edgeMap[info[item.first].req->graph[*first].number][j];
				DDCGraph::Map::edge_descriptor ed = edge(e.first.first, e.first.second, d->graph).first;
				if (d->graph[ed].core[e.second].enable == false) {//enable出ない場合調整
					int coreId = *(d->graph[ed].core[e.second].intCores.begin());
					for (auto enedge : d->initEnableInfoMap[ed]) {//同じリンクのenableなコアを探索
						if (d->graph[ed].core[enedge].intCores.find(coreId) != d->graph[ed].core[enedge].intCores.end()) {//コアを含むものがあれば
							info[item.first].edgeMap[info[item.first].req->graph[*first].number][j] = std::make_pair(e.first, enedge);
							break;
						}
					}
				}
			}
		}
	}


	allocInfo a = {allocId,request,resourceInfo,edgeInfo,partitionData};
	info[allocId]=a;


	return;
}

void AllocateData::removeAllocData(int id, DDCGraph* ddcGraph) {//要素の削除

	auto vertex_range = vertices(info[id].req->graph);
	auto edge_range = edges(info[id].req->graph);


	for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
		ddcGraph->graph[info[id].resourceMap[info[id].req->graph[*first].number]].residual += 1;//リソースの復旧
	}

	map<DDCGraph::Map::edge_descriptor, map<int, bool>> setFlg;
	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
		int a = info[id].req->graph[*first].number;//仮想リンクの識別値
		for (int i = 0; i < info[id].edgeMap[a].size(); i++) {
			DDCGraph::Map::edge_descriptor e = edge(info[id].edgeMap[a][i].first.first, info[id].edgeMap[a][i].first.second, ddcGraph->graph).first;//解放するリンク
			int coreNum = info[id].edgeMap[a][i].second;//解放するコア

			if (setFlg[e][coreNum] == true) {
				continue;
			}

			setFlg[e][coreNum] = true;
			vector<int> eraseNum;


			for (int j = 0; j < ddcGraph->graph[e].core[coreNum].useAppRate.size(); j++) {
				if (info[id].req->reqID == ddcGraph->graph[e].core[coreNum].useAppRate[j].second) {
					eraseNum.push_back(j);
				}
			}

			std::vector<std::pair<std::map<int, double>, int>>::iterator it = ddcGraph->graph[e].core[coreNum].useAppRate.begin();
			while (it != ddcGraph->graph[e].core[coreNum].useAppRate.end()) {
				if (info[id].req->reqID == (*it).second) {
					it = ddcGraph->graph[e].core[coreNum].useAppRate.erase(it);
				}
				else ++it;
			}


			ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.first] = 0;
			ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.second] = 0;
			for (int t = 0; t < ddcGraph->graph[e].core[coreNum].useAppRate.size(); t++) {
				ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.first] +=
					ddcGraph->graph[e].core[coreNum].useAppRate[t].first[info[id].edgeMap[a][i].first.first];
				ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.second] +=
					ddcGraph->graph[e].core[coreNum].useAppRate[t].first[info[id].edgeMap[a][i].first.second];
			}



			if (floor(ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.first] * 100000) <= 0) {
				ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.first] = 0;
				ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.first] = ddcGraph->graph[e].propagation;
			}
			else {
				ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.first] = 0;

				if (ddcGraph->graph[info[id].edgeMap[a][i].first.first].resource == 0) {
					ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.first] = ddcGraph->graph[e].propagation;
				}else {
					ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.first] = ddcGraph->graph[e].propagation;
				}
			}


			if (floor(ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.second] * 100000) <= 0) {
				ddcGraph->graph[e].core[coreNum].rate[info[id].edgeMap[a][i].first.second] = 0;
				ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.second] = ddcGraph->graph[e].propagation;
			}
			else {

				if (ddcGraph->graph[info[id].edgeMap[a][i].first.second].resource == 0) {
					ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.second] = ddcGraph->graph[e].propagation;
				}
				else {
					ddcGraph->graph[e].core[coreNum].delay[info[id].edgeMap[a][i].first.second] = ddcGraph->graph[e].propagation;
				}
			}

			std::vector<int> tmpCMap;
			for (auto encore : ddcGraph->initEnableInfoMap[e]) {
				tmpCMap.push_back(encore);
			}
			

			ddcGraph->initCandInfoMap[e] = tmpCMap;//initcandの更新



		}
	}
	delete info[id].req;//デリーとしておく
	info.erase(id);

	cout << "光パス削除\n";
	ddcGraph->eraseLightPath();

}



