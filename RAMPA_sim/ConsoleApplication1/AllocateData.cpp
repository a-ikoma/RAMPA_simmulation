#include "AllocateData.h"
using namespace std;


AllocateData::AllocateData()
{
	return;
}


void AllocateData::resetAll() {//�S�v�f�̉��
	for (auto &a : info) {
		delete a.second.req;
		a.second.req = nullptr;
	}
}

void AllocateData::pushAllocData(int allocId,Request* request, std::map<int, int> resourceInfo, std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> edgeInfo,DDCGraph* d, std::pair<std::vector<int>, std::pair<double, double>> partitionData) {//���蓖�ď����Z�b�g
	//�G�b�W�}�b�v�̍X�V

	for (const auto& item : info) {
		auto edge_range = edges(info[item.first].req->graph);
		for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {//�X�V
			for (int j = 0; j < info[item.first].edgeMap[info[item.first].req->graph[*first].number].size(); j++) {
				auto e = info[item.first].edgeMap[info[item.first].req->graph[*first].number][j];
				DDCGraph::Map::edge_descriptor ed = edge(e.first.first, e.first.second, d->graph).first;
				if (d->graph[ed].core[e.second].enable == false) {//enable�o�Ȃ��ꍇ����
					int coreId = *(d->graph[ed].core[e.second].intCores.begin());
					for (auto enedge : d->initEnableInfoMap[ed]) {//���������N��enable�ȃR�A��T��
						if (d->graph[ed].core[enedge].intCores.find(coreId) != d->graph[ed].core[enedge].intCores.end()) {//�R�A���܂ނ��̂������
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

void AllocateData::removeAllocData(int id, DDCGraph* ddcGraph) {//�v�f�̍폜

	auto vertex_range = vertices(info[id].req->graph);
	auto edge_range = edges(info[id].req->graph);


	for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
		ddcGraph->graph[info[id].resourceMap[info[id].req->graph[*first].number]].residual += 1;//���\�[�X�̕���
	}

	map<DDCGraph::Map::edge_descriptor, map<int, bool>> setFlg;
	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
		int a = info[id].req->graph[*first].number;//���z�����N�̎��ʒl
		for (int i = 0; i < info[id].edgeMap[a].size(); i++) {
			DDCGraph::Map::edge_descriptor e = edge(info[id].edgeMap[a][i].first.first, info[id].edgeMap[a][i].first.second, ddcGraph->graph).first;//������郊���N
			int coreNum = info[id].edgeMap[a][i].second;//�������R�A

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
			

			ddcGraph->initCandInfoMap[e] = tmpCMap;//initcand�̍X�V



		}
	}
	delete info[id].req;//�f���[�Ƃ��Ă���
	info.erase(id);

	cout << "���p�X�폜\n";
	ddcGraph->eraseLightPath();

}



