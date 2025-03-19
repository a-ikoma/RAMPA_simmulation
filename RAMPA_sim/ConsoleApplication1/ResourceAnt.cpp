#include "ResourceAnt.h"

using namespace std;


void ResourceAnt::threadAnt2(std::map<int, int> rMap, int num, DDCGraph* d, Request* req, double feroP, double costP, std::map<DDCGraph::Map::edge_descriptor, double>* edgeCost) {

    LinkAnt* bestAnt = nullptr;
    double minCost = -1;
    bool sucessFlg = false;

    double baseCost;
    if (bestCost == -1 || d->allocatePolicy == 2) {
        baseCost = 99999999;
    }
    else {
        baseCost = bestCost;
    }

    for (int i = 0; i < linkThreadParam; i++) {
        LinkAnt* a = new LinkAnt(d, req, rMap, hopLimit, feroP, costP, baseFeromon, reqs, edgemaps, resourcemaps, baseCost, allCost);

        if (a->LinkEmbedding(&settingFeromons, d, req, edgeCost) == true) {
            ModelPartition mp = ModelPartition();
            if (mp.ModelPartitioning(a, d, req) == true){
                sucessFlg = true;
                if (a->allCost >= 0 && (minCost == -1 || minCost > a->allCost)) {
                    if (bestAnt != nullptr) {
                        delete bestAnt;
                    }
                    bestAnt = a;
                    minCost = a->allCost;
                    baseCost = minCost;
                }else {
                    delete a;
                }
            }else {
                delete a;
            }
        } else {
            delete a;
        }
    }
    if (bestAnt != nullptr) {
#pragma omp critical
{
        bestAnts[num] = bestAnt;
}
    }
    return;
}



ResourceAnt::ResourceAnt(){
    feromonRate = 0.1;
    allCost = 0;
    feromonRate = 0;
    antSedai = 0;
    antNum = 0;
    feromonParam = 0;
    costParam = 0;
    linkThreadParam = 0;
    searchItr = 0;
    baseFeromon = 0;
}


ResourceAnt::ResourceAnt(DDCGraph* ddcGraph, int sedai, int antnum, double feroP, double costP, double feroRate, int hopLim, 
    std::vector<Request*>* reqsData, std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemapsData, std::vector<std::map<int, int>*>* resourcemapsData, double bestCostData) {


    reqs = reqsData;
    edgemaps = edgemapsData;
    resourcemaps = resourcemapsData;

    bestCost= bestCostData;


    hopLimit = hopLim;
    allCost = 0;
    feromonRate = feroRate;



    antSedai = sedai;
    antNum = antnum;
    feromonParam = feroP;
    costParam = costP;

    linkThreadParam = antNum / 10;
    searchItr = antNum / linkThreadParam;

    auto req_range = vertices(ddcGraph->graph);
    for (auto first = req_range.first, last = req_range.second; first != last; ++first) {//—v‹ƒOƒ‰ƒt
        residualData[ddcGraph->graph[*first].number] = ddcGraph->graph[*first].residual;
    }
    for (int i = 0; i < searchItr; i++) {
        bestAnts[i] = nullptr;
    }
    bestLinkAnt = nullptr;
    baseFeromon = ddcGraph->initialFeromon;
    maxResSearchFlg = false;
}


bool ResourceAnt::ResourceEmbedding(DDCGraph* ddcGraph, Request* req) {

    auto req_range = vertices(req->graph);
    bool setNodeRangeFlg = false;
    std::vector<int> distance;

    
    vector<int> resourceRange;

    if (maxResSearchFlg) {
        resourceRange = ddcGraph->maxResidualNode;
    }
    else {
        resourceRange = ddcGraph->allocateResourceRange;
    }

    std::map<DDCGraph::Map::edge_descriptor, double> edgeCost;

    ddcGraph->updateResourceCost(&resourceCosts, &residualData);


    auto vertex_range = vertices(ddcGraph->graph);

    for (auto vfirst = req_range.first, last = req_range.second; vfirst != last; ++vfirst) {
        vector<double> feromons;
        vector<double> costs;
        vector<DDCGraph::Map::vertex_descriptor> nodes;

        vector<int> resRange2;
        double totalW = 0;

        for (auto efirst : resourceRange) {
            if (residualData[ddcGraph->graph[efirst].number] <= 0 || (bestCost != -1 && bestCost < allCost + resourceCosts[efirst]) || checkRange(ddcGraph->graph[efirst].number, ddcGraph, resourceRange) == false) {
                continue;
            }

            if (maxResSearchFlg == true && ddcGraph->graph[efirst].residual < req->stageNum) {
                continue;
            }

            resRange2.push_back(ddcGraph->graph[efirst].number);

            if (ddcGraph->graph[efirst].resource == req->graph[*vfirst].resource) {

                if (maxResSearchFlg == true) {
                    costs.push_back(1/resourceCosts[efirst]);
                    totalW += pow(ddcGraph->graph[efirst].feromon, feromonParam) * (1 / pow(1/resourceCosts[efirst], costParam));
                }
                else {
                    costs.push_back(resourceCosts[efirst]);
                    totalW += pow(ddcGraph->graph[efirst].feromon, feromonParam) * (1 / pow(resourceCosts[efirst], costParam));
                }


                feromons.push_back(ddcGraph->graph[efirst].feromon);

                nodes.push_back(efirst);
            }
        }

        resourceRange.clear();
        resourceRange = resRange2;

        std::random_device rd;
        std::default_random_engine eng(rd());
        std::uniform_real_distribution<> distr(0, 1);
        double rnd = distr(eng);

        if (feromons.size() == 0) {
            return false;
        }
        double bottom = 0;
        double tmp = 0;
        int selectNode = 0;

        for (int i = 0; i < feromons.size(); i++) {
            tmp = (pow(feromons[i], feromonParam) * (1 / pow(costs[i], costParam))) / totalW;
            if ((rnd >= bottom && rnd <= bottom + tmp) || i == feromons.size() - 1) {
                selectNode = ddcGraph->graph[nodes[i]].number;
                resourceMap[req->graph[*vfirst].number] = selectNode;
                residualData[selectNode] -= 1;
                allCost += resourceCosts[nodes[i]];
                break;
            }
            bottom += tmp;
        }
        ddcGraph->updateResourceCost(&resourceCosts, &residualData);

        selectedNodes.push_back(selectNode);

        if (maxResSearchFlg == true) {
            resourceRange.clear();
            resourceRange = { selectNode };
        }
    }

    double minCost = -1;
    bool sucessFlg = false;


    ddcGraph->updateLinkCost(&edgeCost, &resourceCosts, &residualData);

    int hikakuCount = 0;

    int endCount = 0;
    double preMinCost = -1;



    for (int j = 0; j < antSedai; j++) {
        for (const auto& item : bestAnts) {
            if (item.second != nullptr) {
                delete item.second;
            }
            bestAnts[item.first] = nullptr;
        }

#pragma omp parallel for
        for (int i = 0; i < searchItr; i++) {
            threadAnt2(resourceMap, i, ddcGraph, req, feromonParam, costParam, &edgeCost);
        }


        map<int, bool> results;
        bool result = false;

        for (int i = 0; i < searchItr; i++) {
            if (bestAnts[i] != nullptr) {
                results[i] = true;
                result = true;
                sucessFlg = true;
            }
            else {
                results[i] = false;
            }
        }

        double tmpCost = -1;
        for (int i = 0; i < searchItr; i++) {
            if (j == 1) {
                break;
            }
            if (results[i] == false) {
                continue;
            }


            if (bestAnts[i]->allCost != -1 && (tmpCost == -1 || tmpCost > bestAnts[i]->allCost)) {

                if (bestLinkAnt != nullptr) {
                    delete bestLinkAnt;
                }


                tmpCost = bestAnts[i]->allCost;
                bestLinkAnt = bestAnts[i];
                bestAnts[i] = nullptr;

                minCost = tmpCost;

            }
            else {
                delete bestAnts[i];
                bestAnts[i] = nullptr;
            }
        }

        if (sucessFlg == true) {

            if (ddcGraph->allocatePolicy!=2 && minCost < 1) {
                break;
            }
        }

        setFeromon(ddcGraph, req, result, minCost);



        if (tmpCost != -1 && (preMinCost == -1 || preMinCost == tmpCost)) {
            preMinCost = minCost;
            endCount++;
        }
        else {
            preMinCost = minCost;
            endCount = 0;
        }
        if (endCount > 4) {
            break;
        }

    }

    if (sucessFlg == true) {
        if (ddcGraph->allocatePolicy != 2) {
            allCost += minCost;
        }else {
            allCost = minCost;
        }
    }
    return sucessFlg;
}

void ResourceAnt::setFeromon(DDCGraph* ddcGraph, Request* req, bool sucessFlg, double tmpCost) {
    baseFeromon *= ddcGraph->decreaseRate;
    if (sucessFlg == true && bestLinkAnt != nullptr) {
        auto edge_range2 = edges(req->graph);
        set<std::pair<std::pair<int, int>, int>> LinkSet;
        for (auto vfirst = edge_range2.first, last = edge_range2.second; vfirst != last; ++vfirst) {
            for (auto e : bestLinkAnt->EdgeMap[req->graph[*vfirst].number]) {
                LinkSet.insert(e);
            }
        }
        for (auto e : LinkSet) {
            DDCGraph::Map::edge_descriptor ed = edge(e.first.first, e.first.second, ddcGraph->graph).first;
            double tmp = feromonRate / tmpCost;
            if (settingFeromons.find(ed) == settingFeromons.end()) {
                settingFeromons[ed][e.second] = baseFeromon + tmp;
                continue;
            }

            if (settingFeromons[ed].find(e.second) == settingFeromons[ed].end()) {
                settingFeromons[ed][e.second] = baseFeromon + tmp;
                continue;
            }
            settingFeromons[ed][e.second] *= ddcGraph->decreaseRate;
            settingFeromons[ed][e.second] += tmp;
        }
    }
}



bool ResourceAnt::checkRange(int node, DDCGraph* d, std::vector<int>& resRange) {
    return true;
    bool flg = true;
    for (auto dis : selectedNodes) {
        if (d->distances[dis][node] > 5) {
            flg = false;
            break;
        }
    }
    return flg;
}