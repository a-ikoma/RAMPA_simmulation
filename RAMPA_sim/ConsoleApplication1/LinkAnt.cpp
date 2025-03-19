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

	if (ddcGraph->allocatePolicy == 2) {//rate * D) / c=1
		for (int i = 0; i < 4; i++) {//正規化のため
			maxTraffic[i] = (i + 1) / 0.003;
		}
	}

}

bool LinkAnt::LinkEmbedding(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons, DDCGraph* ddcGraph, Request* req, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts) {

	auto edge_range = edges(req->graph);
	map<int, vector<ReqGraph::ReqMap::edge_descriptor>> proc_vedge;
	vector<ReqGraph::ReqMap::edge_descriptor> conGPU_vedge;
	std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> curLightPath;


	for (auto proc : req->task_proc[1]) {//プロセスごとに一度分類
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

			resourcePairPropagation[b][a] = 0;//資源間の伝搬遅延記録の初期化
			currentResourcePairPropagation = 0;

			std::vector<int> passNode;

			distance = ddcGraph->distances[t];

			tarDistance = ddcGraph->distances[t];
				
			selectLink(settingFeromons, ddcGraph, req, s, t, vedge, passNode, edgeCosts, 0, false, curLightPath, -1);

			resourcePairPropagation[b][a] = currentResourcePairPropagation;
			currentResourcePairPropagation = 0;

			if (missFlg) {//経路は見つからなかった
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

	if (ddcGraph->graph[source].number == ddcGraph->graph[target].number) {//引数はtと等しいか、等しかったら終わり

		if (hops == 0) {//同一プールの資源ボード上のGPUである場合、資源からOCSとOCSから資源への遅延を加算
			DDCGraph::Map::edge_descriptor df = edge(source, ddcGraph->graph[source].adjNode[0], ddcGraph->graph).first;
			currentResourcePairPropagation += ddcGraph->graph[df].propagation + ddcGraph->graph[source].trans_delay + ddcGraph->graph[ddcGraph->graph[source].adjNode[0]].trans_delay + ddcGraph->graph[df].propagation;
		}

		return;
	}
	vector<pair<DDCGraph::Map::edge_descriptor, int>> nallocCand;//未割り当てリンク候補
	vector<pair<DDCGraph::Map::edge_descriptor, int>> allocCand;//割り当てリンク候補


	if (hops >= hopLimit) {
		missFlg = true;
		return;
	}


	if (ddcGraph->graph[source].resource == 4 && inLghtPath == true) {//今回線スイッチかつ、光パスの中にいるとき、
		if (curLightPath.size() == 0) {
			cout << "おかしいnewlightPaths.size()が\n";
			exit(1);
		}
		allocCand.push_back(curLightPath[lightPathHop]);
	}
	else {

		searchCand(ddcGraph, req, source, nallocCand, allocCand, vedge, passNode, edgeCosts, hops, inLghtPath, target);//候補の決定
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
			//埋め込み処理
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

	//決定したリンクの伝搬遅延（リンクの伝搬遅延＋スイッチ遅延）を記録

	currentResourcePairPropagation += ddcGraph->graph[selectedLink].propagation + ddcGraph->graph[source].trans_delay;

	if (ddcGraph->graph[source].resource == 4 && inLghtPath == true) {//現在地が光回線スイッチで光パスに突入しているとき
		selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, curLightPath, lightPathHop + 1);
	}
	else if (ddcGraph->graph[source].resource == 4 && inLghtPath == false) {//現在地が光回線スイッチで光パスに突入していないとき
		newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].push_back({ selectedLink,selectedLinkCore });
		if (ddcGraph->graph[nextSource].resource != 4) {//次のノードが資源かパケットで一度終端される時、逆の光パスも設定しておく
			if (newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].size()!=1) {//1ホップの光パスだとこの処理はひつようないので
				std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>> rLightPath(newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].rbegin(), newlightPaths[curLightPathSourceEdge][curLightPathSourceCore].rend());
				newlightPaths[selectedLink][selectedLinkCore] = rLightPath;
			}
		}
		selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, false, curLightPath, -1);
	}
	else {
		if (selectedFromAllocCand == true) {//パケットまたは資源で、既に設定される光パスを利用するとき
			curLightPathSourceEdge = selectedLink;//現在の光パスのソースリンクとコアを保存
			curLightPathSourceCore = selectedLinkCore;
			bool motokaraFlg = false;
			if ((*tmp_lightPaths).find(selectedLink) != (*tmp_lightPaths).end()) {
				if ((*tmp_lightPaths)[selectedLink].find(selectedLinkCore) != (*tmp_lightPaths)[selectedLink].end()) {//元から設定されていた光パスの場合
					selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, (*tmp_lightPaths)[selectedLink][selectedLinkCore], 1);
					motokaraFlg = true;
				}
			}
			if (motokaraFlg == false) {
				selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, true, newlightPaths[selectedLink][selectedLinkCore], 1);
			}
		}
		else {//パケットまたは資源で、新しい光パスを設定するとき
			newlightPaths[selectedLink][selectedLinkCore] = { {selectedLink,selectedLinkCore} };//新しい光パスを追加
			curLightPathSourceEdge = selectedLink;//現在の光パスのソースリンクとコアを保存
			curLightPathSourceCore = selectedLinkCore;
			std::vector < std::pair<DDCGraph::Map::edge_descriptor, int>> tmpLight;//仮置きのからの集合
			selectLink(settingFeromons, ddcGraph, req, nextSource, target, vedge, passNode, edgeCosts, hops + 1, false, tmpLight, -1);
		}
	}
}


void LinkAnt::searchCand(DDCGraph* ddcGraph, Request* req, int source,
	vector<pair<DDCGraph::Map::edge_descriptor, int>>& nallocCand, vector<pair<DDCGraph::Map::edge_descriptor, int>>& allocCand,
	ReqGraph::ReqMap::edge_descriptor vedge, std::vector<int> passNode, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts, int hops, bool inLghtPath, int curTarget) {

	DDCGraph::Map::edge_descriptor e;
	vector<pair<DDCGraph::Map::edge_descriptor, vector<int>>> candEdge;//リンクとそのコア番号の組を候補として保存していく


	int mode = -1;//以下の３パターンを識別する値

	if (ddcGraph->graph[source].resource == 4 && inLghtPath == false) {//今、光回線スイッチにいて、特定の光パスにいないとき、
		//未割当リンクの中から自由に選びましょう（割り当て済みリンクを排除した状態で今までと一緒）
		mode = 1;

	}else if (ddcGraph->graph[source].resource != 4) {//資源またはパケットスイッチにいるとき、
		//自由に選択しよう(ただし、特定の資源へつながる光パスへつながる場合は注意しよう)
		mode = 2;
	}

	if (mode == 1) {//資源へとつながるリンクを選択するのはmode1の時のみ、なぜなら資源は回線スイッチとのみつながるから
		//そして、未割り当てリンクしか使えない。なぜなら光パス関係ないから
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

			//ターゲットのノードが一回のホップで行けるとき、選択的に選ぶ必要はないので、リソースに行くまでのコアだけ選ぶという処理をここでする。
			if (resourceMap[req->graph[vedge].adjNode[0]] == ddcGraph->graph[source].adjNode[i]
				|| resourceMap[req->graph[vedge].adjNode[1]] == ddcGraph->graph[source].adjNode[i]) {//宛先ノードが隣接ノードにある場合s
				e = boost::edge(ddcGraph->graph[source].number, ddcGraph->graph[source].adjNode[i], ddcGraph->graph).first;//隣接リンクを取得

				bool ocupyFlg = false;

				if (ocupyFlg == false && (*nallocCandinfo).find(e) != (*nallocCandinfo).end()) {
					for (const auto& item : (*nallocCandinfo)[e]) {
						if (linkant_newAllocatedLink.find(e) != linkant_newAllocatedLink.end()) {
							if (linkant_newAllocatedLink[e].find(item.first) != linkant_newAllocatedLink[e].end()) {
								continue;//もうナロックではないから
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

		if (distance[ddcGraph->graph[source].adjNode[i]] + hops > hopLimit) {//ホップの上限超えてたら
			continue;
		}

		bool ocupyFlg = false;
		e = boost::edge(ddcGraph->graph[source].number, ddcGraph->graph[source].adjNode[i], ddcGraph->graph).first;//隣接リンクを取得

		if ((*allocCandinfo).find(e) != (*allocCandinfo).end() && mode == 2) {//候補となる割り当て済みリンクがあるということ

			for (const auto& item : (*allocCandinfo)[e]) {//* + (distance[ddcGraph->graph[source].adjNode[i]] * ddcGraph->graph[e].propagation)*/ 
				//ここは元から設定されていた割り当て済みリンクなので(*tmp_lightPaths)をそのまま使っても大丈夫
				if (item.second == true) {
					DDCGraph::Map::edge_descriptor tmpE = (*tmp_lightPaths)[e][item.first][(*tmp_lightPaths)[e][item.first].size() - 1].first;
					if ((*tmp_lightPaths)[e][item.first].size() <= 1) {
						cout << "おかしいはず" << "\n";
						exit(1);
					}

					int dst1 = ddcGraph->graph[tmpE].adjNode[0];
					int dst2 = ddcGraph->graph[tmpE].adjNode[1];

					int endNode;//光パスの終端ノード

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
						(ddcGraph->graph[dst2].resource != 4 && ddcGraph->graph[dst2].resource != 5) && (dst2 != resourceMap[req->graph[vedge].adjNode[0]] && dst2 != resourceMap[req->graph[vedge].adjNode[1]])) {//光パスで関係ない資源へ行っちゃうとき
						continue;
					}


					if ((checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
						continue;
					}



					tmpACand[e].push_back(item.first);
					if (ddcGraph->graph[e].core[item.first].useAppRate.size() == 0 && ddcGraph->graph[e].core[item.first].coreNum == 1) {//占有している割り当て済みリンクがあれば未割り当てリンクをあえて割り当てる必要はないので
						ocupyFlg = true;
						break;
					}
				}
			}
		}

		if (newAllocCandinfo.find(e) != newAllocCandinfo.end() && mode == 2) {

			for (const auto& item : newAllocCandinfo[e]) {//* + (distance[ddcGraph->graph[source].adjNode[i]] * ddcGraph->graph[e].propagation)*/ 

				if (item.second == true) {

					DDCGraph::Map::edge_descriptor tmpE = newlightPaths[e][item.first][newlightPaths[e][item.first].size() - 1].first;

					int dst1 = ddcGraph->graph[tmpE].adjNode[0];
					int dst2 = ddcGraph->graph[tmpE].adjNode[1];

					int endNode;//光パスの終端ノード

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
						(ddcGraph->graph[dst2].resource != 4 && ddcGraph->graph[dst2].resource != 5) && (dst2 != resourceMap[req->graph[vedge].adjNode[0]] && dst2 != resourceMap[req->graph[vedge].adjNode[1]])) {//光パスで関係ない資源へ行っちゃうとき
						continue;
					}

					if ((checkOcupy(ddcGraph, req, e, item.first) == false || ddcGraph->graph[e].core[item.first].coreNum > 1 || ddcGraph->graph[e].core[item.first].useAppRate.size() != 0) == false) {
						continue;
					}

					tmpACand[e].push_back(item.first);
					if (ddcGraph->graph[e].core[item.first].useAppRate.size() == 0 && ddcGraph->graph[e].core[item.first].coreNum == 1) {//占有している割り当て済みリンクがあれば未割り当てリンクをあえて割り当てる必要はないので
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
						continue;//もうナロックではないから
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


void LinkAnt::initCandInfo(DDCGraph* ddcGraph, Request* req) {//割り当て候補enbleとallocInfoの初期設定
	//DDCGraphが情報を持っておくかたち
	tmp_lightPaths = &(ddcGraph->lightPaths);//ライトパス情報をこぴー
	allocCandinfo = &(ddcGraph->allocCandinfo);
	nallocCandinfo = &(ddcGraph->nallocCandinfo);
	enableinfo = &(ddcGraph->enableinfo);

}

void LinkAnt::embIntLink(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum,
	int source, int target, ReqGraph::ReqMap::edge_descriptor vedge) {//統合リンクと割り当て済みリンクが選択されたとき

	bool firstFlg = false;
	if (newAppRate[e][coreNum].first[source] == 0 && newAppRate[e][coreNum].first[target] == 0) {//どっちも０だから初めてこのリンクに埋め込み
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


	if (ddcGraph->graph[e].core[coreNum].coreNum <= 1 || ddcGraph->graph[e].core[coreNum].enable == true) {//統合リンクではない
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

	if (firstFlg == false) {//一回入ってるから
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
		


		//tDelay[target] = responseTime(rate2, ddcGraph->graph[e].core[coreNum].coreNum, ddcGraph->graph[target].trans_delay, ddcGraph->graph[target].cutthrough_delay, ddcGraph->graph[e].core[coreNum].useAppRate.size()) + ddcGraph->graph[e].propagation;
		for (auto coreOfInt : coreList[e][coreNum]) {
			checkDelay[e][coreOfInt] = tDelay;//エッジマップの調整面倒だからここで代入しておく
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

	cout << "統合を禁止しているからここにはこないはず。いったんparam.txtをチェック";
	exit(1);



}

void LinkAnt::updateCandInfo(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum,
	int source, ReqGraph::ReqMap::edge_descriptor vedge) {//未割当のリンクを割り当てたときに候補となる統合リンクを探す

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

void LinkAnt::updateRate(DDCGraph* ddcGraph, Request* req) {//割り当て情報をもとにレート情報を更新
	auto edge_range = edges(req->graph);
	DDCGraph::Map::edge_descriptor ed;

	map<DDCGraph::Map::edge_descriptor, map<int, bool>> edgeFlg;
	for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {//更新
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
	DDCGraph::Map::edge_descriptor e, int coreNum) {//フェロモンの値を取得
	if ((*settingFeromons).find(e) == (*settingFeromons).end()) {//存在しないなら
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