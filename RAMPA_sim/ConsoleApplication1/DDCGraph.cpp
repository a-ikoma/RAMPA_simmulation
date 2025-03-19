#include "DDCGraph.h"


using namespace std;
const long long INF = 1LL << 60;


DDCGraph::DDCGraph(std::string filename, double gensui, double initFeromon, int allocPolicy) {//コンストラクタ

    allocatePolicy = allocPolicy;
    file = filename;
    string graphName = "";
    initialFeromon = initFeromon;
    decreaseRate = gensui;
    graph = createGraph();
    auto edge_range = edges(graph);
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        for (int i = 0; i < graph[*first].core.size(); i++) {//未割当リンク
            int id = i;
            initCandInfoMap[*first].push_back(id);
            initEnableInfoMap[*first].push_back(id);
        }
    }

    auto vertex_range = vertices(graph);

    int nodeCnt = 0;
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        nodeCnt++;
        if (graph[*first].resource == 1) {
            cout << "ノード" << graph[*first].number << "(メモリ)" << "の残余は";
            cout << graph[*first].residual << "\n";
        }
        else if (graph[*first].resource == 2) {
                cout << "ノード" << graph[*first].number << "(CPU)" << "の残余は";
                cout << graph[*first].residual << "\n";
        }
        else if (graph[*first].resource == 3) {
            cout << "ノード" << graph[*first].number << "(GPU)" << "の残余は";
            cout << graph[*first].residual << "\n";
        }
        else {
            cout << "ノード" << graph[*first].number << "(スイッチ)" << "の残余は";
        }


    }
    allNodeNum = nodeCnt;

    CPUResidual = 0;
    GPUResidual = 0;
    memResidual = 0;

    for (auto efirst = vertex_range.first, elast = vertex_range.second; efirst != elast; ++efirst) {//基盤グラフ
        std::vector<int> dis(nodeCnt, 0);
        deriveNodeDistance(graph[*efirst].number, dis);
        distances[graph[*efirst].number] = dis;

        if (graph[*efirst].resource == 1) {
            memResidual += graph[*efirst].residual;
        }
        else if (graph[*efirst].resource == 2) {
            CPUResidual += graph[*efirst].residual;
        }
        else if (graph[*efirst].resource == 3) {
            GPUResidual += graph[*efirst].residual;
        }


    }
    setAllocateResourceRange();



    for (int i = 0; i < gpuIter.size(); i++) {


        for (int j = 0; j < gpuIter.size(); j++) {
            if (i == j) {
                continue;
            }
            bool existFlg = false;
            for (int t = 0; t < gpuPairs.size(); t++) {
                if ((gpuIter[i] == gpuPairs[t].first || gpuIter[i] == gpuPairs[t].second) && (gpuIter[j] == gpuPairs[t].first || gpuIter[j] == gpuPairs[t].second)) {
                    existFlg = true;
                    break;
                }
            }
            if (existFlg == false) {
                gpuPairs.push_back({ gpuIter[i],gpuIter[j] });
            }
        }
    }




    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
#pragma omp parallel for
        for (int i = 0; i < gpuIter.size(); i++) {
            map<int, long long> x;
            map<int, long long> x1;
            map<int, int> y;
            map<int, int> z;
            map<int, int> tmpedgeCount;

            derive_shortest_path(gpuIter[i], graph[*first].adjNode[0], graph[*first].adjNode[1], x, x1, y, z, tmpedgeCount);
#pragma omp critical
            {

                shortestHop[*first][gpuIter[i]] = x;
                shortestHopExceptEdge[*first][gpuIter[i]] = x1;
                shortestPaths[*first][gpuIter[i]] = z;
                shortestPathsExceptEdge[*first][gpuIter[i]] = tmpedgeCount;
            }
        }

#pragma omp parallel for
        for (int i = 0; i < packetIter.size(); i++) {
            map<int, long long> x;
            map<int, long long> x1;
            map<int, int> y;
            map<int, int> z;
            map<int, int> tmpedgeCount;

            derive_shortest_path(packetIter[i], graph[*first].adjNode[0], graph[*first].adjNode[1], x, x1, y, z, tmpedgeCount);
#pragma omp critical
            {
                shortestHop[*first][packetIter[i]] = x;
                shortestHopExceptEdge[*first][packetIter[i]] = x1;
                shortestPaths[*first][packetIter[i]] = z;//（）
                shortestPathsExceptEdge[*first][packetIter[i]] = tmpedgeCount;
            }
        }

    }


    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        if (graph[graph[*first].adjNode[0]].resource != 4 || graph[graph[*first].adjNode[1]].resource != 4 ||
            graph[graph[*first].adjNode[0]].resource != 5 || graph[graph[*first].adjNode[1]].resource != 5) {
            continue;
        }
        swLinks.push_back(*first);
    }


    for (const auto& item : initCandInfoMap) {
        for (auto i : item.second) {//コアのvectorを持つから
            if (graph[item.first].core[i].rate[graph[item.first].adjNode[1]] != 0 ||
                graph[item.first].core[i].rate[graph[item.first].adjNode[0]] != 0
                || graph[item.first].core[i].intCores.size() > 1) {
                allocCandinfo[item.first][i] = true;
            }
            else {
                nallocCandinfo[item.first][i] = true;
            }
        }

    }

    for (const auto& item : initEnableInfoMap) {
        for (auto i : item.second) {//コアのvectorを持つから
            enableinfo[item.first][i] = true;
        }
    }


    std::map<int, std::pair<int, int>> flops_maxRes;//GPUの種類ごとの残余の最大値

    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (graph[*first].resource != 0) {
            if (graph[*first].resource == 3) {
                if (flops_maxRes.find(graph[*first].flops) != flops_maxRes.end()) {
                    if (flops_maxRes[graph[*first].flops].second < graph[*first].residual) {
                        flops_maxRes[graph[*first].flops] = { graph[*first].number, graph[*first].residual };
                    }
                }
                else {
                    flops_maxRes[graph[*first].flops] = { graph[*first].number, graph[*first].residual };
                }
            }

        }
    }

    for (auto const& item : flops_maxRes) {
        maxResidualNode.clear();
        maxResidualNode.push_back(item.second.first);
    }
}

DDCGraph::DDCGraph() {
    return;
}

vector<string> split_naive(const string& s, char delim) {
    vector<string> elems;
    string item;
    for (char ch : s) {
        if (ch == delim) {
            if (!item.empty())
                elems.push_back(item);
            item.clear();
        }
        else {
            item += ch;
        }
    }
    if (!item.empty())
        elems.push_back(item);
    return elems;
}

DDCGraph::Map DDCGraph::createGraph() {
    Map map;
    map[boost::graph_bundle].name = "DDCGraph";
    int count = 0;

    string data;
    std::ifstream ifs(file);
    if (!ifs) {
        std::cout << "ファイルが開けませんでした。" << std::endl;
        std::cin.get();
        return map;
    }

    std::string buf;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        data = buf;
        if (data.find("link_sw") != std::string::npos) {//リンクの要素定義
            vector<string> link = split_naive(data, ',');//[0]:行、[1]：列
            propagationValue = stod(link[1].c_str());

        }
        else if (data.find("link_resource") != std::string::npos) {//リンクの要素定義
            vector<string> link = split_naive(data, ',');//[0]:行、[1]：列
            propagationValue_resource = stod(link[1].c_str());

        }
        else if (data.find("name") != std::string::npos) {
            vector<string> cores = split_naive(data, ',');//[0]:行、[1]：列
            graphName = cores[1];
        }
        else if (data.find("bw") != std::string::npos) {//帯域幅（GBps)
            bandwidth = stod(split_naive(data, ',')[1]);
        }
        else if (data.find("Csw") != std::string::npos) {//光回線スイッチのノード
            vector<string> sw = split_naive(data, ',');
            int swP = 3;
            for (int i = 0; i < atoi(sw[1].c_str()); i++) {
                Map::vertex_descriptor v1 = add_vertex(map);
                map[v1].number = count;
                map[v1].resource = 4;
                map[v1].block = 0;
                map[v1].residual = 0;
                map[v1].trans_delay = stod(sw[2].c_str());
                circuitDelay = map[v1].trans_delay;
                map[v1].cost = 1;
                map[v1].flops = 0;
                map[v1].feromon = initialFeromon;
                map[v1].capacity = 0;
                map[v1].memBand = 0;
                count++;
            }
            for (int i = swP; i < sw.size(); i++) {
                vector<string> sw_link = split_naive(sw[i], '-');
                Map::edge_descriptor e;
                bool inserted = false;
                int v1 = atoi(sw_link[0].c_str());
                int v2 = atoi(sw_link[1].c_str());
                int linkNum = atoi(sw_link[2].c_str());
                boost::tie(e, inserted) = add_edge(v1, v2, map);
                map[v1].adjNode.push_back(map[v2].number);
                map[v2].adjNode.push_back(map[v1].number);
                map[e].adjNode.push_back(map[v1].number);
                map[e].adjNode.push_back(map[v2].number);
                map[e].coreNumber = linkNum;
                std::map<int, double> tmp;
                tmp[map[v2].number] = 0;
                tmp[map[v1].number] = 0;

                for (int j = 0; j < linkNum; j++) {

                    coreInfo a = { true,true,1, tmp ,1 ,{j} };
                    a.delay[map[v2].number] = propagationValue;//伝播遅延を挿入しておく
                    a.delay[map[v1].number] = propagationValue;//伝播遅延を挿入しておく
                    map[e].core[j] = a;
                }

            }

        }
        else if (data.find("gpu") != std::string::npos) {//演算リソースのノード
            vector<string> calc = split_naive(data, ',');
            int swP = 7;
            int lastAdjNode = -1;

            for (int i = 0; i < atoi(calc[1].c_str()); i++) {

                Map::vertex_descriptor v1 = add_vertex(map);
                map[v1].number = count;
                map[v1].resource = 3;
                map[v1].block = atoi(calc[2].c_str());
                map[v1].residual = atoi(calc[2].c_str());
                GPUAllResidual = atoi(calc[2].c_str());
                map[v1].trans_delay = stod(calc[3].c_str());
                map[v1].flops = stod(calc[4].c_str());
                map[v1].cost = (map[v1].residual) * map[v1].flops;
                map[v1].feromon = initialFeromon;
                map[v1].capacity = stod(calc[5].c_str());
                map[v1].memBand = stod(calc[6].c_str());

                count++;
                auto vertex_range = vertices(map);
                for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {//リソースとスイッチのリンク
                    if (map[*first].number == atoi(calc[swP].c_str())) {


                        Map::edge_descriptor e;
                        bool inserted = false;
                        boost::tie(e, inserted) = add_edge(*first, v1, map);
                        if (lastAdjNode != map[*first].number) {
                            allocateResourceRange.push_back(map[v1].number);
                            lastAdjNode = map[*first].number;
                        }


                        map[v1].adjNode.push_back(map[*first].number);
                        map[*first].adjNode.push_back(map[v1].number);
                        map[e].adjNode.push_back(map[v1].number);
                        map[e].adjNode.push_back(map[*first].number);
                        map[e].coreNumber = 32;//資源プールとのリンクは資源プールのパケットスイッチで処理できるコネクション数と資源プールモジュール内の資源数をもとに、必要な分だけ

                        std::map<int, double> tmp;
                        tmp[map[*first].number] = 0;
                        tmp[map[v1].number] = 0;

                        for (int j = 0; j < map[e].coreNumber; j++) {

                            coreInfo a = { true,true,1, tmp ,1 ,{j} };
                            a.delay[map[*first].number] = propagationValue_resource;//伝播遅延を挿入しておく
                            a.delay[map[v1].number] = propagationValue_resource;//伝播遅延を挿入しておく
                            map[e].core[j] = a;
                        }

                        swP++;
                        break;
                    }
                }
                gpuIter.push_back(v1);
                //packetIter.push_back(v1);
            }
        }

    }


    //リンクのパラメータ設定
    auto edge_range = edges(map);
    int count2 = 0;
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        Map::edge_descriptor e = *first;
        map[e].cost = 1;

        if (map[map[e].adjNode[0]].resource != 4 || map[map[e].adjNode[0]].resource != 4 ||
            map[map[e].adjNode[1]].resource != 5 || map[map[e].adjNode[1]].resource != 5) {//資源とつながるリンクの場合
            map[e].propagation = propagationValue_resource;
        }
        else {
            map[e].propagation = propagationValue;
        }


        map[e].number = count2;
        map[e].passWeight = 1;

        count2++;

    }


    return map;
}


void DDCGraph::updateResourceCost(std::map<Map::vertex_descriptor, double>* resourceCosts, std::map<int, int>* residualData) {
    auto vertex_range = vertices(graph);

    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (graph[*first].resource == 1) {//メモリ
            (*resourceCosts)[*first] = (double)(*residualData)[graph[*first].number];;

        }else if (graph[*first].resource == 2) {//CPU

            (*resourceCosts)[*first] = ((double)(*residualData)[graph[*first].number] * graph[*first].flops);
        }
        else if (graph[*first].resource == 3) {//GPU
            if (allocatePolicy == 2) {
                (*resourceCosts)[*first] = 1/((graph[*first].flops * 0.001) * graph[*first].capacity)+1;
            }else {
                //(*resourceCosts)[*first] =  (graph[*first].flops *0.001)* graph[*first].capacity;//TFLOPS*GB
                (*resourceCosts)[*first] = (graph[*first].flops * 0.001) * graph[*first].capacity* (double)(*residualData)[graph[*first].number];//TFLOPS*GB
            }

            
        }

    }
}




void DDCGraph::updateLinkCost(std::map<Map::edge_descriptor, double>* edgeCosts, std::map<Map::vertex_descriptor, double>* resourceCosts, std::map<int, int>* resData) {

    auto edge_range = edges(graph);
    int count = 0;


    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        double cost = 0;

        if (allocatePolicy == 2) {
            (*edgeCosts)[*first] = 1;
            continue;
        }

        for (int i = 0; i < gpuPairs.size(); i++) {
            if (graph[gpuPairs[i].first].residual <= 0 || graph[gpuPairs[i].second].residual <= 0) {//このペアは通信しない
                continue;
            }

            double prod = 1;

            double rc1 = (*resourceCosts)[gpuPairs[i].first];

            double rc2 = (*resourceCosts)[gpuPairs[i].second];
            if (shortestHop[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number] == shortestHopExceptEdge[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) {//そのリンクを通らない経路で最短経路が存在
                cost += prod*(((double)shortestPaths[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number] - (double)shortestPathsExceptEdge[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) / (double)shortestPaths[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) * ((rc1 + rc2) / (double)shortestHop[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]);
            }
            else if (shortestHop[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number] < shortestHopExceptEdge[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) {//そのリンクを通らない経路で最短経路は存在しない
                cost += prod*(((double)shortestPaths[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) / (double)shortestPaths[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]) * ((rc1 + rc2) / (double)shortestHop[*first][gpuPairs[i].first][graph[gpuPairs[i].second].number]);
            }
        }
        (*edgeCosts)[*first] = cost;
    }


}



void DDCGraph::decreaseNodeFeromon() {//ノードのフェロモンを減衰
    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        graph[*first].feromon = graph[*first].feromon * decreaseRate;
    }
}


void DDCGraph::derive_shortest_path(Map::vertex_descriptor s, int edge1, int edge2,
    std::map<int, long long>& dis, std::map<int, long long>& dis2,
    std::map<int, int>& prev,
    std::map<int, int>& count, map<int, int>& edgeCount) {// sは開始点
    //Qは頂点の集合（もしくは優先度付きキュー）。
    //u, v は頂点。d(v) はスタートとなる頂点からの最短経路の長さ。prev(v)は最短経路をたどる際の前の頂点。
    priority_queue<pair<long, int>, vector<pair<long, int>>, greater<pair<long, int>>> Q;
    priority_queue<pair<long, int>, vector<pair<long, int>>, greater<pair<long, int>>> Q2;
    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (graph[s].number == graph[*first].number) {
            dis[graph[*first].number] = 0;
            dis2[graph[*first].number] = 0;
            count[graph[*first].number] = 1;
            edgeCount[graph[*first].number] = 1;
        }
        else {
            dis[graph[*first].number] = INF;
            dis2[graph[*first].number] = INF;
            count[graph[*first].number] = 0;
            edgeCount[graph[*first].number] = 0;
        }
    }
    Q.emplace(dis[graph[s].number], graph[s].number);
    Q2.emplace(dis2[graph[s].number], graph[s].number);
    while (!Q.empty()) {
        pair<long, int> p = Q.top();
        Q.pop();
        int v = p.second;
        if (dis[v] < p.first) {
            continue;
        }
        for (auto v1 : graph[v].adjNode) {


            if (dis[v1] > dis[v] + 1) {
                dis[v1] = dis[v] + 1;
                prev[v1] = v; // 頂点 v を通って e.to にたどり着いた
                Q.emplace(dis[v1], v1);
                count[v1] = count[v];
            }
            else if (dis[v1] == dis[v] + 1) {
                count[v1] += count[v];
            }
        }
    }

    while (!Q2.empty()) {
        pair<long, int> p = Q2.top();
        Q2.pop();
        int v = p.second;
        if (dis2[v] < p.first) {
            continue;
        }

        for (auto v1 : graph[v].adjNode) {

            if ((v1 == edge1 || v1 == edge2) && (v == edge1 || v == edge2)) {
                continue;
            }

            if (dis2[v1] > dis2[v] + 1) {
                dis2[v1] = dis2[v] + 1;
                Q2.emplace(dis2[v1], v1);
                edgeCount[v1] = edgeCount[v];
            }
            else if (dis2[v1] == dis2[v] + 1) {
                edgeCount[v1] += edgeCount[v];
            }
        }
    }

}




void DDCGraph::deriveNodeDistance(Map::vertex_descriptor s, std::vector<int>& dis) {// sは開始点
    //Qは頂点の集合（もしくは優先度付きキュー）。
    //u, v は頂点。d(v) はスタートとなる頂点からの最短経路の長さ。prev(v)は最短経路をたどる際の前の頂点。
    priority_queue<pair<long, int>, vector<pair<long, int>>, greater<pair<long, int>>> Q;
    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (graph[s].number == graph[*first].number) {
            dis[graph[*first].number] = 0;
        }
        else {
            dis[graph[*first].number] = 100000;
        }
    }
    Q.emplace(dis[graph[s].number], graph[s].number);
    while (!Q.empty()) {
        pair<long, int> p = Q.top();
        Q.pop();
        int v = p.second;
        if (dis[v] < p.first) {
            continue;
        }
        for (auto v1 : graph[v].adjNode) {

            if (dis[v1] > dis[v] + 1) {
                dis[v1] = dis[v] + 1;
                Q.emplace(dis[v1], v1);
            }
        }
    }
}


int DDCGraph::calcCoreMapID(std::set<int> cores) {//コアの添え字を構成するコアの番号から導出する（最大値は16として）
    int id = 0;
    for (auto x : cores) {
        id += x;
    }
    return id;
}


void DDCGraph::resetFeromon() {
    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        graph[*first].feromon = initialFeromon;
    }
    return;
}



void DDCGraph::setResidual(std::map<int, int> residualData) {
    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        graph[*first].residual = residualData[graph[*first].number];
    }
}


void DDCGraph::setAllocateResourceRange() {

    auto vertex_range = vertices(graph);
    allocateResourceRange.clear();
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (graph[*first].residual > 0) {
            allocateResourceRange.push_back(graph[*first].number);
        }
    }
}


void DDCGraph::eraseLightPath() {//ライトパスを削除

    for (int i = 0; i < gpuIter.size(); i++) {
        for (auto s : graph[gpuIter[i]].adjNode) {
            Map::edge_descriptor e = edge(gpuIter[i], s, graph).first;
            for (int c = 0; c < graph[e].coreNumber; c++) {
                if (graph[e].core[c].useAppRate.size() == 0) {
                    lightPaths[e].erase(c);
                }
            }
            if (lightPaths[e].size() == 0) {
                lightPaths.erase(e);
            }
        }
    }


}


void DDCGraph::setAllocCandInfo() {
    allocCandinfo.clear();
    nallocCandinfo.clear();

    for (const auto& item : initCandInfoMap) {
        for (auto i : item.second) {//コアのvectorを持つから
            if (graph[item.first].core[i].rate[graph[item.first].adjNode[1]] != 0 ||
                graph[item.first].core[i].rate[graph[item.first].adjNode[0]] != 0
                || graph[item.first].core[i].intCores.size() > 1) {
                allocCandinfo[item.first][i] = true;
            }
            else {
                nallocCandinfo[item.first][i] = true;
            }
        }

    }
}