#include "LinkAnt.h"


using namespace std;



LinkAnt::LinkAnt() {

	hopLimit = -1;
	allCost = 0;
	feromonParam = 0;
	costParam = 0;

	baseFeromon = 0;

}


LinkAnt::LinkAnt(DDCGraph* ddcGraph, Request* req, std::map<int, int> map, int limit, double feroP, double costP, double baseFero,
	std::vector<Request*>* reqsData, std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemapsData, std::vector<std::map<int, int>*>* resourcemapsData, double baseCostData, double allCostData) {

	reqs = reqsData;
	edgemaps = edgemapsData;
	resourcemaps = resourcemapsData;
	baseCost = baseCostData;
	allocatedResourceCost = allCostData;

	resourceMap = map;
	hopLimit = limit;

	allCost = 0;
	feromonParam = feroP;
	costParam = costP;

	baseFeromon = baseFero;
	initCandInfo(ddcGraph, req);


	baseCost = 9999999;

	if (ddcGraph->allocatePolicy == 2) {
		for (int i = 0; i < 4; i++) {
			maxTraffic[i] = (i + 1) / 0.003;
		}
	}

}

bool LinkAnt::LinkEmbedding(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons, DDCGraph* ddcGraph, Request* req, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts) {

	auto edge_range = edges(req->graph);
	map<int, vector<ReqGraph::ReqMap::edge_descriptor>> proc_vedge;
	vector<ReqGraph::ReqMap::edge_descriptor> conGPU_vedge;
	std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> curLightPath;


	for (auto proc : req->task_proc[1]) {
		for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {

			if (req->graph[*first].process[0] == proc) {
				proc_vedge[proc].push_back(*first);
			}
		}
	}


	for (auto proc : req->task_proc[1]) {
		for (const auto vedge : proc_vedge[proc]) {
			int s, t;
			int a = req->graph[vedge].adjNode[0];
			int b = req->graph[vedge].adjNode[1];

			int proc = req->graph[vedge].process[0];
			s = resourceMap[b];
			t = resourceMap[a];

			resourcePairPropagation[b][a] = 0;
			currentResourcePairPropagation = 0;

			std::vector<int> passNode;

			distance = ddcGraph->distances[t];

			tarDistance = ddcGraph->distances[t];
				
			selectLink(settingFeromons, ddcGraph, req, s, t, vedge, passNode, edgeCosts, 0, false, curLightPath, -1);

			resourcePairPropagation[b][a] = currentResourcePairPropagation;
			currentResourcePairPropagation = 0;

			if (missFlg) {
				return false;
			}

		}

	}

	map<DDCGraph::Map::edge_descriptor, map<int, bool>> edgeFlg2;

	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
		for (int j = 0; j < EdgeMap[req->graph[*first].number].size(); j++) {
			auto e = EdgeMap[req->graph[*first].number][j];
			DDCGraph::Map::edge_descriptor edgeID = edge(e.first.first, e.first.second, ddcGraph->graph).first;
			if (edgeFlg2[edgeID][e.second] == false) {
				newAppRate[edgeID][e.second].first[e.first.first] = 0;
				newAppRate[edgeID][e.second].first[e.first.second] = 0;
				newAppRate[edgeID][e.second].second = req->reqID;
			}
			edgeFlg2[edgeID][e.second] = true;
			newAppRate[edgeID][e.second].first[e.first.first] += req->graph[*first].read_rate;
			newAppRate[edgeID][e.second].first[e.first.second] += req->graph[*first].write_rate;
		}
	}


	map<DDCGraph::Map::edge_descriptor, map<int, bool>> edgeFlg;
	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
		for (int j = 0; j < EdgeMap[req->graph[*first].number].size(); j++) {
			auto e = EdgeMap[req->graph[*first].number][j];
			DDCGraph::Map::edge_descriptor edgeID = edge(e.first.first, e.first.second, ddcGraph->graph).first;
			if (edgeFlg[edgeID][e.second] == false) {

				newAppRate[edgeID][e.second].second = req->reqID;

				useAppRates[edgeID][e.second].push_back(newAppRate[edgeID][e.second]);
				edgeFlg[edgeID][e.second] = true;
			}
		}
	}
	return true;
}

void LinkAnt::selectLink(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons, DDCGraph* ddcGraph,
	Request* req, int source, int target, ReqGraph::ReqMap::edge_descriptor vedge, std::vector<int>& passNode,
	std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts, int hops, bool inLghtPath, std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> curLightPath, int lightPathHop) {

	passNode.push_back(source);

	if (ddcGraph->graph[source].number == ddcGraph->graph[target].number) {

		if (hops == 0) {
			DDCGraph::Map::edge_descriptor df = edge(source, ddcGraph->graph[source].adjNode[0], ddcGraph->graph).first;
			currentResourcePairPropagation += ddcGraph->graph[df].propagation + ddcGraph->graph[source].trans_delay + ddcGraph->graph[ddcGraph->graph[source].adjNode[0]].trans_delay + ddcGraph->graph[df].propagation;
		}

		return;
	}
	vector<pair<DDCGraph::Map::edge_descriptor, int>> nallocCand;
	vector<pair<DDCGraph::Map::edge_descriptor, int>> allocCand;


	if (hops >= hopLimit) {
		missFlg = true;
		return;
	}


	if (ddcGraph->graph[source].resource == 4 && inLghtPath == true) {
		if (curLightPath.size() == 0) {
			cout << "おかしいnewlightPaths.size()が\n";
			exit(1);
		}
		allocCand.push_back(curLightPath[lightPathHop]);
	}
	else {

		searchCand(ddcGraph, req, source, nallocCand, allocCand, vedge, passNode, edgeCosts, hops, inLghtPath, target);
	}

	if ((nallocCand.size() == 0 && allocCand.size() == 0) || hops >= hopLimit) {
		missFlg = true;
		return;
	}


	//選択的決定

	double totalW = 0;
	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<> distr(0, 1);
	double rnd = distr(eng);
	for (int i = 0; i < nallocCand.size(); i++) {
		if (ddcGraph->allocatePolicy == 2) {
			totalW += (pow(feromonValue(settingFeromons, nallocCand[i].first, nallocCand[i].second), feromonParam)) * (1 / (1 + pow(distance[target] / 4, costParam)));
		}
		else {
			totalW += (pow(feromonValue(settingFeromons, nallocCand[i].first, nallocCand[i].second), feromonParam)) * (1 / (1 + pow((*edgeCosts)[nallocCand[i].first], costParam)));
		}
	}

	for (int i = 0; i < allocCand.size(); i++) {
		if (ddcGraph->allocatePolicy == 2) {
			double linkcost = (tmpDelay[allocCand[i].first][allocCand[i].second][source] / maxTraffic[ddcGraph->graph[allocCand[i].first].core[allocCand[i].second].coreNum]) + (distance[target] / 4) + 1;
			totalW += (pow(feromonValue(settingFeromons, allocCand[i].first, allocCand[i].second), feromonParam))
				* (1 / linkcost, costParam);
		}
		else {
			totalW += pow(feromonValue(settingFeromons, allocCand[i].first, allocCand[i].second), feromonParam);
		}
	}

	double bottom = 0;
	double tmp = 0;
	bool endFlg = false;


	int selectedLinkCore = -1;
	DDCGraph::Map::edge_descriptor selectedLink;


	int nextSource = 0;
	bool selectedFromAllocCand = false;
	for (int i = 0; i < nallocCand.size(); i++) {
		if (ddcGraph->allocatePolicy == 2) {
			tmp += ((pow(feromonValue(settingFeromons, nallocCand[i].first, nallocCand[i].second), feromonParam)) * (1 / (1 + pow(distance[target] / 4, costParam)))) / totalW;
		}
		else {
			tmp = ((pow(feromonValue(settingFeromons, nallocCand[i].first, nallocCand[i].second), feromonParam)) * (1 / (1 + pow((*edgeCosts)[nallocCand[i].first], costParam)))) / totalW;
		}

		if ((rnd >= bottom && rnd <= bottom + tmp) || (allocCand.size() == 0 && i == nallocCand.size() - 1)) {
			int a = ddcGraph->graph[source].number;
			if (a == ddcGraph->graph[nallocCand[i].first].adjNode[0]) {
				updateCandInfo(ddcGraph, req, nallocCand[i].first, nallocCand[i].second, source, vedge);
				nextSource = ddcGraph->graph[nallocCand[i].first].adjNode[1];
				EdgeMap[req->graph[vedge].number].push_back(std::make_pair(make_pair(source, nextSource), nallocCand[i].second));
			}
			else {
				updateCandInfo(ddcGraph, req, nallocCand[i].first, nallocCand[i].second, source, vedge);
				nextSource = ddcGraph->graph[nallocCand[i].first].adjNode[0];
				EdgeMap[req->graph[vedge].number].push_back(std::make_pair(make_pair(source, nextSource), nallocCand[i].second));
			}
			endFlg = true;

			double rate, rate2;


			rate = req->graph[vedge].read_rate;
			rate2 = req->graph[vedge].write_rate;

			double latency;


			latency = ddcGraph->graph[nallocCand[i].first].propagation + ddcGraph->graph[source].trans_delay;

			std::map<int, double> tDelay;
			tDelay[source] = latency;


			tDelay[nextSource] = ddcGraph->graph[nallocCand[i].first].propagation + ddcGraph->graph[nextSource].trans_delay;
			

			checkDelay[nallocCand[i].first][nallocCand[i].second] = tDelay;
			
			if (ddcGraph->allocatePolicy != 2) {
				allCost += (*edgeCosts)[nallocCand[i].first];
			}
			else {
				allCost += distance[target] / 4;
			}

			selectedLink = nallocCand[i].first;
			selectedLinkCore = nallocCand[i].second;

			break;
		}
		bottom += tmp;
	}

	if (endFlg == false) {
		selectedFromAllocCand = true;
		for (int i = 0; i < allocCand.size(); i++) {

			double linkcost;
			if (ddcGraph->allocatePolicy == 2) {
				linkcost = (tmpDelay[allocCand[i].first][allocCand[i].second][source] / maxTraffic[ddcGraph->graph[allocCand[i].first].core[allocCand[i].second].coreNum]) + (distance[target] / 4) + 1;
				tmp = ((pow(feromonValue(settingFeromons, allocCand[i].first, allocCand[i].second), feromonParam))
					* (1 / linkcost, costParam)) / totalW;
			}
			else {
				tmp = (pow(feromonValue(settingFeromons, allocCand[i].first, allocCand[i].second), feromonParam)) / totalW;
			}

			if ((rnd >= bottom && rnd <= bottom + tmp) || i == allocCand.size() - 1) {
				int a = ddcGraph->graph[source].number;
				if (a == ddcGraph->graph[allocCand[i].first].adjNode[0]) {

					nextSource = ddcGraph->graph[allocCand[i].first].adjNode[1];
					embIntLink(ddcGraph, req, allocCand[i].first, allocCand[i].second, source, nextSource, vedge);
					EdgeMap[req->graph[vedge].number].push_back(std::make_pair(make_pair(source, nextSource), allocCand[i].second));
				}
				else {

					nextSource = ddcGraph->graph[allocCand[i].first].adjNode[0];
					embIntLink(ddcGraph, req, allocCand[i].first, allocCand[i].second, source, nextSource, vedge);
					EdgeMap[req->graph[vedge].number].push_back(std::make_pair(make_pair(source, nextSource), allocCand[i].second));
				}
				endFlg = true;
				if (ddcGraph->allocatePolicy == 2) {
					allCost += linkcost;
				}

				selectedLink = allocCand[i].first;
				selectedLinkCore = allocCand[i].second;

				break;
			}
			bottom += tmp;
		}
	}

	currentResourcePairPropagation += ddcGraph->graph[selectedLink].propagation + ddcGraph->graph[source].trans_delay;

	if (ddcGraph->graph[source].resource == 4 && inLghtPath == true) {
		selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, curLightPath, lightPathHop + 1);
	}
	else if (ddcGraph->graph[source].resource == 4 && inLghtPath == false) {
		newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].push_back({ selectedLink,selectedLinkCore });
		if (ddcGraph->graph[nextSource].resource != 4) {
			if (newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].size()!=1) {
				std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>> rLightPath(newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].rbegin(), newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].rend());
				newlightPaths[selectedLink][selectedLinkCore] = rLightPath;
			}
		}
		selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, false, curLightPath, -1);
	}
	else {
		if (selectedFromAllocCand == true) {
			curLightPathSourceEdge = selectedLink;
			curLightPathSourceCore = selectedLinkCore;
			bool motokaraFlg = false;
			if ((*tmp_lightPaths).find(selectedLink) != (*tmp_lightPaths).end()) {
				if ((*tmp_lightPaths)[selectedLink].find(selectedLinkCore) != (*tmp_lightPaths)[selectedLink].end()) {
					selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, (*tmp_lightPaths)[selectedLink][selectedLinkCore], 1);
					motokaraFlg = true;
				}
			}
			if (motokaraFlg == false) {
				selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, newlightPaths[selectedLink][selectedLinkCore], 1);
			}
		}
		else {
			newlightPaths[selectedLink][selectedLinkCore] = { {selectedLink,selectedLinkCore} };
			curLightPathSourceEdge = selectedLink;
			curLightPathSourceCore = selectedLinkCore;
			std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> tmpLight;
			selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, false, tmpLight, -1);
		}
	}
}


void LinkAnt::searchCand(DDCGraph* ddcGraph, Request* req, int source,
	vector<pair<DDCGraph::Map::edge_descriptor, int>>& nallocCand, vector<pair<DDCGraph::Map::edge_descriptor, int>>& allocCand,
	ReqGraph::ReqMap::edge_descriptor vedge, std::vector<int> passNode, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts, int hops, bool inLghtPath, int curTarget) {

	DDCGraph::Map::edge_descriptor e;
	vector<pair<DDCGraph::Map::edge_descriptor, vector<int>>> candEdge;


	int mode = -1;//以下の３パターンを識別する値

	if (ddcGraph->graph[source].resource == 4 && inLghtPath == false) {
		mode = 1;

	}else if (ddcGraph->graph[source].resource != 4) {
		mode = 2;
	}

	if (mode == 1) {
		bool oneFindFlg = false;
		for (int i = 0; i < ddcGraph->graph[source].adjNode.size(); i++) {
			bool passFlg = false;

			for (auto passedNode : passNode) {
				if (passedNode == ddcGraph->graph[source].adjNode[i]) {
					passFlg = true;
				}
			}
			if (passFlg == true) {
				continue;
			}

			if (ddcGraph->graph[ddcGraph->graph[source].adjNode[i]].resource == 5) {
				e = boost::edge(ddcGraph->graph[source].number, ddcGraph->graph[source].adjNode[i], ddcGraph->graph).first;
				for (const auto& item : (*nallocCandinfo)[e]) {
					if (linkant_newAllocatedLink.find(e) != linkant_newAllocatedLink.end()) {
						if (linkant_newAllocatedLink[e].find(item.first) != linkant_newAllocatedLink[e].end()) {
							continue;//もうナロックではないから
						}
					}
					if (item.second == true && baseCost > allCost + allocatedResourceCost + (*edgeCosts)[e] && 
						(checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {

						nallocCand.clear();

						nallocCand.push_back(std::make_pair(e, item.first));
						return;
					}
					else {
						break;
					}
				}
			}

			if (oneFindFlg == true) {
				continue;
			}

			map<DDCGraph::Map::edge_descriptor, vector<int>> tmpACand;
			map<DDCGraph::Map::edge_descriptor, vector<int>> tmpNCand;

			if (resourceMap[req->graph[vedge].adjNode[0]] == ddcGraph->graph[source].adjNode[i]
				|| resourceMap[req->graph[vedge].adjNode[1]] == ddcGraph->graph[source].adjNode[i]) {
				e = boost::edge(ddcGraph->graph[source].number, ddcGraph->graph[source].adjNode[i], ddcGraph->graph).first;

				bool ocupyFlg = false;

				if (ocupyFlg == false && (*nallocCandinfo).find(e) != (*nallocCandinfo).end()) {
					for (const auto& item : (*nallocCandinfo)[e]) {
						if (linkant_newAllocatedLink.find(e) != linkant_newAllocatedLink.end()) {
							if (linkant_newAllocatedLink[e].find(item.first) != linkant_newAllocatedLink[e].end()) {
								continue;
							}
						}
						if (item.second == true && baseCost > allCost + allocatedResourceCost + (*edgeCosts)[e] &&
							(checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
							if (oneFindFlg == false) {
								tmpNCand[e].push_back(item.first);
								oneFindFlg = true;
							}
							break;
						}
					}
				}

				for (const auto& item : tmpNCand) {
					for (auto c : item.second) {
						nallocCand.push_back(std::make_pair(item.first, c));
					}
				}
			}
		}
		if (oneFindFlg) {
			return;
		}
	}

	map<DDCGraph::Map::edge_descriptor, vector<int>> tmpACand;
	map<DDCGraph::Map::edge_descriptor, vector<int>> tmpNCand;
	for (int i = 0; i < ddcGraph->graph[source].adjNode.size(); i++) {

		bool passFlg = false;

		for (auto passedNode : passNode) {
			if (passedNode == ddcGraph->graph[source].adjNode[i]) {
				passFlg = true;
				break;
			}
		}
		if (passFlg == true) {
			continue;
		}

		if (distance[ddcGraph->graph[source].adjNode[i]] + hops > hopLimit) {
			continue;
		}

		bool ocupyFlg = false;
		e = boost::edge(ddcGraph->graph[source].number, ddcGraph->graph[source].adjNode[i], ddcGraph->graph).first;

		if ((*allocCandinfo).find(e) != (*allocCandinfo).end() && mode == 2) {

			for (const auto& item : (*allocCandinfo)[e]) {
				if (item.second == true) {
					DDCGraph::Map::edge_descriptor tmpE = (*tmp_lightPaths)[e][item.first][(*tmp_lightPaths)[e][item.first].size() - 1].first;
					if ((*tmp_lightPaths)[e][item.first].size() <= 1) {
						cout << "おかしいはず" << "\n";
						exit(1);
					}

					int dst1 = ddcGraph->graph[tmpE].adjNode[0];
					int dst2 = ddcGraph->graph[tmpE].adjNode[1];

					int endNode;

					if (ddcGraph->graph[dst1].resource != 4) {
						endNode = dst1;
					}
					else {
						endNode = dst2;
					}

					if (hops + (*tmp_lightPaths)[e][item.first].size() + distance[endNode] > hopLimit) {
						continue;
					}

					if (((ddcGraph->graph[dst1].resource != 4 && ddcGraph->graph[dst1].resource != 5) && (dst1 != resourceMap[req->graph[vedge].adjNode[0]] && dst1 != resourceMap[req->graph[vedge].adjNode[1]])) ||
						(ddcGraph->graph[dst2].resource != 4 && ddcGraph->graph[dst2].resource != 5) && (dst2 != resourceMap[req->graph[vedge].adjNode[0]] && dst2 != resourceMap[req->graph[vedge].adjNode[1]])) {
						continue;
					}


					if ((checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
						continue;
					}



					tmpACand[e].push_back(item.first);
					if (ddcGraph->graph[e].core[item.first].useAppRate.size() == 0 && ddcGraph->graph[e].core[item.first].coreNum == 1) {
						ocupyFlg = true;
						break;
					}
				}
			}
		}

		if (newAllocCandinfo.find(e) != newAllocCandinfo.end() && mode == 2) {

			for (const auto& item : newAllocCandinfo[e]) {

				if (item.second == true) {

					DDCGraph::Map::edge_descriptor tmpE = newlightPaths[e][item.first][newlightPaths[e][item.first].size() - 1].first;

					int dst1 = ddcGraph->graph[tmpE].adjNode[0];
					int dst2 = ddcGraph->graph[tmpE].adjNode[1];

					int endNode;

					if (ddcGraph->graph[dst1].resource != 4) {
						endNode = dst1;
					}
					else {
						endNode = dst2;
					}

					if (hops + newlightPaths[e][item.first].size() + distance[endNode] > hopLimit) {
						continue;
					}

					if (((ddcGraph->graph[dst1].resource != 4 && ddcGraph->graph[dst1].resource != 5) && (dst1 != resourceMap[req->graph[vedge].adjNode[0]] && dst1 != resourceMap[req->graph[vedge].adjNode[1]])) ||
						(ddcGraph->graph[dst2].resource != 4 && ddcGraph->graph[dst2].resource != 5) && (dst2 != resourceMap[req->graph[vedge].adjNode[0]] && dst2 != resourceMap[req->graph[vedge].adjNode[1]])) {
						continue;
					}

					if ((checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
						continue;
					}

					tmpACand[e].push_back(item.first);
					if (ddcGraph->graph[e].core[item.first].useAppRate.size() == 0 && ddcGraph->graph[e].core[item.first].coreNum == 1) {
						ocupyFlg = true;
						break;
					}
				}
			}
		}

		if (ocupyFlg == false && (*nallocCandinfo).find(e) != (*nallocCandinfo).end()) {

			for (const auto& item : (*nallocCandinfo)[e]) {

				if (linkant_newAllocatedLink.find(e) != linkant_newAllocatedLink.end()) {
					if (linkant_newAllocatedLink[e].find(item.first) != linkant_newAllocatedLink[e].end()) {
						continue;
					}
				}

				if (item.second == true && baseCost > allCost + allocatedResourceCost + (*edgeCosts)[e] &&
				(checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
					
					tmpNCand[e].push_back(item.first);
					break;
				}
			}
		}
	}



	for (const auto& item : tmpACand) {
		for (auto c : item.second) {
			allocCand.push_back(std::make_pair(item.first, c));
		}
	}

	for (const auto& item : tmpNCand) {
		for (auto c : item.second) {
			nallocCand.push_back(std::make_pair(item.first, c));
		}
	}

	return;
}


void LinkAnt::initCandInfo(DDCGraph* ddcGraph, Request* req) {
	tmp_lightPaths = &(ddcGraph->lightPaths);
	allocCandinfo = &(ddcGraph->allocCandinfo);
	nallocCandinfo = &(ddcGraph->nallocCandinfo);
	enableinfo = &(ddcGraph->enableinfo);

}

void LinkAnt::embIntLink(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum,
	int source, int target, ReqGraph::ReqMap::edge_descriptor vedge) {

	bool firstFlg = false;
	if (newAppRate[e][coreNum].first[source] == 0 && newAppRate[e][coreNum].first[target] == 0) {
		firstFlg = true;
		newAppRate[e][coreNum].first[source] = req->graph[vedge].read_rate;
		newAppRate[e][coreNum].first[target] = req->graph[vedge].write_rate;
		newAppRate[e][coreNum].second = req->reqID;
	}
	else {
		newAppRate[e][coreNum].first[source] += req->graph[vedge].read_rate;
		newAppRate[e][coreNum].first[target] += req->graph[vedge].write_rate;
		newAppRate[e][coreNum].second = req->reqID;
	}


	if (ddcGraph->graph[e].core[coreNum].coreNum <= 1 || ddcGraph->graph[e].core[coreNum].enable == true) {
		checkAllocApp(ddcGraph, e, coreNum);
		if (useAppRates[e][coreNum].size() == 0 && ddcGraph->graph[e].core[coreNum].useAppRate.size() != 0) {
			useAppRates[e][coreNum] = ddcGraph->graph[e].core[coreNum].useAppRate;
		}
		tmpRate[e][coreNum][source] = newAppRate[e][coreNum].first[source];
		tmpRate[e][coreNum][target] = newAppRate[e][coreNum].first[target];
		for (int t = 0; t < useAppRates[e][coreNum].size(); t++) {
			tmpRate[e][coreNum][source] += useAppRates[e][coreNum][t].first[source];
			tmpRate[e][coreNum][target] += useAppRates[e][coreNum][t].first[target];
		}

		double rate, rate2;
		rate = tmpRate[e][coreNum][source];
		rate2 = tmpRate[e][coreNum][target];

		double late;

		late = ddcGraph->graph[e].propagation + ddcGraph->graph[source].trans_delay;
		


		std::map<int, double> tDelay;
		tDelay[source] = late;


		tDelay[target] = ddcGraph->graph[e].propagation + ddcGraph->graph[target].trans_delay;
		


		checkDelay[e][coreNum] = tDelay;
		

		if (ddcGraph->allocatePolicy == 2) {
			if (ddcGraph->graph[e].core[coreNum].useAppRate.size() == 0 && ddcGraph->graph[e].core[coreNum].coreNum == 1) {
				tmpDelay[e][coreNum][source] = 0;
				tmpDelay[e][coreNum][target] = 0;
			}
			else {

				tmpDelay[e][coreNum][source] = rate / ddcGraph->graph[e].core[coreNum].coreNum;
				tmpDelay[e][coreNum][target] = rate2 / ddcGraph->graph[e].core[coreNum].coreNum;
			}

		}


		return;
	}

	if (firstFlg == false) {
		tmpRate[e][coreNum][source] = newAppRate[e][coreNum].first[source];
		tmpRate[e][coreNum][target] = newAppRate[e][coreNum].first[target];
		for (int t = 0; t < useAppRates[e][coreNum].size(); t++) {
			tmpRate[e][coreNum][source] += useAppRates[e][coreNum][t].first[source];
			tmpRate[e][coreNum][target] += useAppRates[e][coreNum][t].first[target];
		}
		double rate, rate2;
		rate = tmpRate[e][coreNum][source];
		rate2 = tmpRate[e][coreNum][target];


		double late;

		late = ddcGraph->graph[e].propagation + ddcGraph->graph[source].trans_delay;
		

		std::map<int, double> tDelay;
		tDelay[source] = late;

		tDelay[target] = ddcGraph->graph[e].propagation + ddcGraph->graph[target].trans_delay;
		
		for (auto coreOfInt : coreList[e][coreNum]) {
			checkDelay[e][coreOfInt] = tDelay;
		}
		checkDelay[e][coreNum] = tDelay;
		

		if (ddcGraph->allocatePolicy == 2) {
			if (ddcGraph->graph[e].core[coreNum].useAppRate.size() == 0 && ddcGraph->graph[e].core[coreNum].coreNum == 1) {
				tmpDelay[e][coreNum][source] = 0;
				tmpDelay[e][coreNum][target] = 0;
			}
			else {
				tmpDelay[e][coreNum][source] = rate;
				tmpDelay[e][coreNum][target] = rate2;
				
			}
		}
		return;
	}

}

void LinkAnt::updateCandInfo(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum,
	int source, ReqGraph::ReqMap::edge_descriptor vedge) {

	linkant_newAllocatedLink[e][coreNum] = true;
	newAllocCandinfo[e][coreNum] = true;


	int target = 0;
	if (source == ddcGraph->graph[e].adjNode[0]) {
		target = ddcGraph->graph[e].adjNode[1];
	}
	else {
		target = ddcGraph->graph[e].adjNode[0];
	}

	newAppRate[e][coreNum].first[source] = req->graph[vedge].read_rate;
	newAppRate[e][coreNum].first[target] = req->graph[vedge].write_rate;
	newAppRate[e][coreNum].second = req->reqID;
	tmpRate[e][coreNum][source] = req->graph[vedge].read_rate;
	tmpRate[e][coreNum][target] = req->graph[vedge].write_rate;


	if (ddcGraph->allocatePolicy == 2) {
		tmpDelay[e][coreNum][source] = 0;
		tmpDelay[e][coreNum][target] = 0;
	}

	vector<set<int>> intEdge;
	vector<int> intEdgeCore;


	if (intEdge.size() <= 1) {
		return;
	}



}

void LinkAnt::updateRate(DDCGraph* ddcGraph, Request* req) {
	auto edge_range = edges(req->graph);
	DDCGraph::Map::edge_descriptor ed;

	map<DDCGraph::Map::edge_descriptor, map<int, bool>> edgeFlg;
	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
		for (auto e : EdgeMap[req->graph[*first].number]) {
			ed = edge(e.first.first, e.first.second, ddcGraph->graph).first;
			if (edgeFlg[ed][e.second] == true) {
				continue;
			}
			if (ddcGraph->allocatePolicy != 2 && ddcGraph->graph[e.first.first].resource != 5 && ddcGraph->graph[e.first.second].resource != 5) {//パケットスイッチと接続するリンクは0.01たさない
				allCost += 0.0001;
			}
		}
	}
}


double LinkAnt::feromonValue(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,
	DDCGraph::Map::edge_descriptor e, int coreNum) {
	if ((*settingFeromons).find(e) == (*settingFeromons).end()) {
		return baseFeromon;
	}

	if ((*settingFeromons)[e].find(coreNum) == (*settingFeromons)[e].end()) {
		return baseFeromon;
	}

	return (*settingFeromons)[e][coreNum];

}


bool LinkAnt::checkOcupy(DDCGraph* d, Request* req, DDCGraph::Map::edge_descriptor e, int core) {
	int id = req->reqID;
	for (auto tmp : d->graph[e].core[core].intCores) {
		for (const auto& encore : (*enableinfo)[e]) {
			if (encore.second == true && d->graph[e].core[encore.first].intCores.find(tmp) != d->graph[e].core[encore.first].intCores.end()) {
				for (auto useapp : d->graph[e].core[encore.first].useAppRate) {
					if (useapp.second != id) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

void LinkAnt::checkAllocApp(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core) {
	for (auto useapp : d->graph[e].core[core].useAppRate) {
		appIDs.insert(useapp.second);
	}

}