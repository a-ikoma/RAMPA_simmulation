#include "AntColony.h"



using namespace std;

//map<int,ResourceAnt> bestAnts;
void AntColony::threadAnt(int ants, int num, DDCGraph* d, Request* req, double feroP, double costP, int curSedai) {

    ResourceAnt bestAnt;

    double minCost = -1;
    bool sucessFlg = false;

    for (int i = 0; i < ants; i++) {
        ResourceAnt ant = ResourceAnt(d, antSedai, antNum, feroP, costP, feromonRate,req->getHopLimit(), &reqs, &edgemaps, &resourcemaps, bestCost);
        if (curSedai == 1 && curFoundFlg == false) {//解が見つからなかったら・・・
            ant.maxResSearchFlg = true;//<-初期収束しすぎるからなし
        }
        if (ant.ResourceEmbedding(d, req) == true) {//候補だったら
            if (ant.bestLinkAnt != nullptr && (minCost == -1 || minCost > ant.allCost)) {
                if (minCost != -1) {
                    delete bestAnt.bestLinkAnt;
                    bestAnt.bestLinkAnt = nullptr;
                }
                bestAnt = ant;
                sucessFlg = true;
                minCost = ant.allCost;
            }
        }else {
            delete ant.bestLinkAnt;
        }
    }
#pragma omp critical
    {
        if (minCost != -1) {
            bestAnts[num] = bestAnt;
        }
        results[num] = sucessFlg;
    }
}

AntColony::AntColony(AllocateData* alloc, int sedai, int antnum, double gensui, double feroP, double costP, int threadNumber, double feroRate) {
    allocData = alloc;

    antSedai = sedai;//蟻の世代数
    antNum = antnum;//１世代の蟻の数
    feromonGensui = gensui;//フェロモンの減衰率
    feromonParam = feroP;//フェロモンの重要度パラメータ
    costParam = costP;//コストの重要度パラメータ
    threads = threadNumber;
    bestCost = -1;
    feromonRate = feroRate;
    syuusokuNum = 20;
    curFoundFlg = false;

    for (int i = 0; i < threads; i++) {
        results.push_back( false);
    }
}
AntColony::AntColony() {
    return;
}

bool AntColony::procEmb(DDCGraph* d, Request* req) {

    //最終的に割り当て結果
    bool result = false;
    int endCount = 0;
    for (auto t : req->tasks) {
        cout<<req->acceptableTime<<"\n";
    }
    double preMinCost = 0;

    auto start = std::chrono::system_clock::now(); // 計測開始時間Time:15991

    auto req_range = vertices(d->graph);
    for (auto first = req_range.first, last = req_range.second; first != last; ++first) {//要求グラフ
        residualData[d->graph[*first].number] = d->graph[*first].residual;
    }//残余数の初期化
    std::map<DDCGraph::Map::vertex_descriptor, double> tmp;//反町君の修論の評価専用だけど、資源工とは一律だからとりあえずダミーとしてだけ用意


    for (int t = 0; t < allocData->info.size(); t++) {//過去の割り当て情報
        edgemaps.push_back(&(allocData->info[t].edgeMap));
        reqs.push_back(allocData->info[t].req);
        resourcemaps.push_back(&(allocData->info[t].resourceMap));
    }

    d->setAllocCandInfo();

    int maxStage = 1;
    int initialStage = 1;

    if (d->allocatePolicy != 0) {//ステージ数を事前決定
        maxStage = req->model->defineStage;
        initialStage = req->model->defineStage;
    }else {//
        //残りのGPU数とオペレーションの数のうちの最小値がマックス
        maxStage = min(d->GPUResidual,req->model->opeNum);

        cout << "max:" << maxStage << "\n";
        initialStage = 1;
    }


    for (int stage = initialStage; stage <= maxStage; stage++) {//各ステージに対して資源を割り当てていきます
        //ステージ数ごとの要求生成

        cout << "ステージ数：" << stage<<"\n";
        req->makeRequestFromStages(stage);//リクエストを再構築

        double tmpCost = -1;
        bool finFlg = false;
        if (result == true) {//前のステージで解を見つけられた場合、次のステージの探索が終わり次第終了する。（コスト的に解は見つからないだろう）
            finFlg = true;
        }


        for (int i = 0; i < antSedai; i++) {////世代数

            cout << i << "世代：";

            int antLoopNum = antNum / threads;


            for (int i = 0; i < results.size(); i++) {
                results[i] = false;
            }

#pragma omp parallel for
            for (int j = 0; j < threads; j++) {
                // ループ変数idは必ずコピーキャプチャする
                threadAnt(antLoopNum, j, d, req, feromonParam, costParam, i);
            }

            for (int j = 0; j < threads; j++) {
                if (results[j] == true) {
                    result = true;
                    curFoundFlg = true;
                }
            }

            
            for (int j = 0; j < threads; j++) {
                if (results[j] == false) {
                    continue;
                }

                if (tmpCost == -1 || tmpCost > bestAnts[j].allCost) {
                    if (tmpCost != -1) {
                        delete bestEmb.bestLinkAnt;
                    }
                    tmpCost = bestAnts[j].allCost;
                    bestEmb = bestAnts[j];
                }
                else {
                    delete bestAnts[j].bestLinkAnt;
                }
            }

            if (bestEmb.bestLinkAnt != nullptr) {
                auto req_range = vertices(req->graph);


                if ((bestCost == -1 && tmpCost != -1) || (tmpCost != -1 && bestCost > tmpCost)) {//パケット優先
                    bestCost = tmpCost;
                    updateLinkAntInfo();

                    if (finFlg) {
                        finFlg = false;
                    }
                }

            }

            d->decreaseNodeFeromon();//フェロモン減衰(1世代分のすべてが決まった後なのでフェロモンを直接更新できる)
            if (result == true) {
                auto req_range = vertices(req->graph);
                set<int> nodeList;
                for (auto vfirst = req_range.first, last = req_range.second; vfirst != last; ++vfirst) {//要求グラフ
                    nodeList.insert(bestEmb.resourceMap[req->graph[*vfirst].number]);
                }

                for (auto v : nodeList) {
                    d->graph[v].feromon += feromonRate / bestCost;
                }
            }
            delete bestEmb.bestLinkAnt;
            bestEmb.bestLinkAnt = nullptr;

            cout << tmpCost << ", ";

        }
        cout << "\n";
        d->resetFeromon();

        if (finFlg == true) {
            break;
        }
    }


    auto end = std::chrono::system_clock::now();// 計測終了時刻を保存
    auto dur = end - start;
    allocationTime = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count(); //処理に要した時間をミリ秒に変換
    cout << "かかった時間:" << allocationTime << "\n";





    if (result == true) {
        d->setResidual(residualData);//残余の更新



        req->makeRequestFromStages(optStage);//リクエストを再構築

        updateNewGraph(d, req);

        cout << "[";
        for (int count : partitionSol.first) {
            cout << count << " ";
        }
        cout << "]";
        cout << ", time:" << partitionSol.second.first << ", throughput:" << partitionSol.second.second;
        cout << endl;


    }


    d->CPUResidual = 0;
    d->GPUResidual = 0;
    d->memResidual = 0;

    auto vertex_range = vertices(d->graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
        if (d->graph[*first].resource == 1) {
            //cout << "ノード" << d->graph[*first].number << "(メモリ)" << "の残余は";
            d->memResidual += d->graph[*first].residual;
        }
        else if (d->graph[*first].resource == 2) {
            d->CPUResidual += d->graph[*first].residual;
        }
        else if (d->graph[*first].resource == 3) {
            d->GPUResidual += d->graph[*first].residual;
        }
    }


    return result;

}

void AntColony::updateLinkAntInfo() {//bestLinkAnt.allCost
    allocCandinfo.clear();
    nallocCandinfo.clear();
    enableinfo.clear();
    rate_delay.clear();
    EdgeMap.clear();
    residualData.clear();
    resourceMap.clear();
    useAppRates.clear();
    HukanouLightPath.clear();
    lightPaths.clear();
    for (const auto& item : bestEmb.bestLinkAnt->newAllocCandinfo) {//割り当てられたリンクのコア全部が割り当てられなくなっちゃってるという問題
        allocCandinfo[item.first] = item.second;
    }
    for (const auto& item : bestEmb.bestLinkAnt->linkant_newAllocatedLink) {
        nallocCandinfo[item.first] = item.second;
    }

    for (const auto& item : bestEmb.bestLinkAnt->rate_delay) {
        rate_delay[item.first] = item.second;
    }
    for (const auto& item : bestEmb.bestLinkAnt->EdgeMap) {
        EdgeMap[item.first] = item.second;
    }
    for (const auto& item : bestEmb.bestLinkAnt->useAppRates) {
        useAppRates[item.first] = item.second;
        
    }
    for (const auto& item : bestEmb.residualData) {
        residualData[item.first] = item.second;
    }
    for (const auto& item : bestEmb.resourceMap) {
        resourceMap[item.first] = item.second;
    }

    for (const auto& item : bestEmb.bestLinkAnt->newlightPaths) {
        lightPaths[item.first] = item.second;
    }

    partitionSol= bestEmb.bestLinkAnt->partitionSol;//モデル分割方法の解

    optStage = bestEmb.bestLinkAnt->partitionSol.first.size();//ステージ数
    relatedAppNum = bestEmb.bestLinkAnt->appIDs.size();
}

void AntColony::updateNewGraph(DDCGraph* d, Request* req) {//グラフに対して遅延とレート、enable、candFlgを更新
    std::map<DDCGraph::Map::edge_descriptor, std::vector<int>> newInitCMap;//candFlgがtrueのエッジディスクリプターとコアをもつ
    std::map<DDCGraph::Map::edge_descriptor, std::vector<int>> newInitEMap;//enableFlgの初期値だけを持つやつ
    auto edge_range = edges(req->graph);
    DDCGraph::Map::edge_descriptor ed;
    map<DDCGraph::Map::edge_descriptor, map<int, bool>> setFlg;





    int outStageNum = -1;
    double dataPeriod = req->model->throughPut;//データが出力される間隔
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {//更新

        double traffic = (req->model->outDataSize[partitionSol.first[req->graph[req->graph[*first].adjNode[1]].number] - 1] / dataPeriod);


        for (auto e : EdgeMap[req->graph[*first].number]) {
          
            ed = edge(e.first.first, e.first.second, d->graph).first;
            if (setFlg[ed][e.second] == true) {
                continue;
            }

            setFlg[ed][e.second] = true;
            d->graph[ed].core[e.second].enable = d->enableinfo[ed][e.second];
            if (d->enableinfo[ed][e.second] == false) {
                cout << "\n\n\n\nここが表示されることある？\n\n\n";
                d->enableinfo[ed][e.second] = true;
            }

            d->graph[ed].core[e.second].rate[e.first.first] = traffic;//トラヒックは一報校にしか起きない
            d->graph[ed].core[e.second].rate[e.first.second] = 0;//トラヒックは一報校にしか起きないだから、0
            if (d->graph[e.first.first].resource != 5) {//パケットじゃなかったら
                d->graph[ed].core[e.second].delay[e.first.first] = d->graph[ed].propagation + d->graph[e.first.first].trans_delay;
            }else {//そうじゃないとき(いったんOCSのみを想定するのでそのまま)
                d->graph[ed].core[e.second].delay[e.first.first] = d->graph[ed].propagation + d->graph[e.first.first].trans_delay;
            }

            if (d->graph[e.first.second].resource != 5) {//パケットじゃなかったら
                d->graph[ed].core[e.second].delay[e.first.second] = d->graph[ed].propagation + d->graph[e.first.second].trans_delay;
            }
            else {
                d->graph[ed].core[e.second].delay[e.first.second] = d->graph[ed].propagation + d->graph[e.first.second].trans_delay;
            }
           

            d->graph[ed].core[e.second].useAppRate = useAppRates[ed][e.second];
        }
    }



    auto req_range = vertices(req->graph);

    for (const auto& item : lightPaths) {
        for (const auto& item2 : lightPaths[item.first]) {
            d->lightPaths[item.first][item2.first] = lightPaths[item.first][item2.first];
        }
    }


    auto edge_range2 = edges(d->graph);
    for (auto first = edge_range2.first, last = edge_range2.second; first != last; ++first) {
        if (d->allocCandinfo.find(*first) != d->allocCandinfo.end()) {
            for (const auto& item : d->allocCandinfo[*first]) {//もとからあるアロックキャンド
                if (item.second == true) {
                    d->graph[*first].core[item.first].candFlg = true;
                    newInitCMap[*first].push_back(item.first);

                }
            }
        }

        if (allocCandinfo.find(*first) != allocCandinfo.end()) {
            for (const auto& item : allocCandinfo[*first]) {//新規のアロックキャンド
                if (item.second == true) {
                    d->graph[*first].core[item.first].candFlg = true;
                    newInitCMap[*first].push_back(item.first);
                }
            }
        }


        if (d->nallocCandinfo.find(*first) != d->nallocCandinfo.end()) {
            for (const auto& item : d->nallocCandinfo[*first]) {
                if (nallocCandinfo.find(*first) != nallocCandinfo.end()) {
                    if (nallocCandinfo[*first].find(item.first) != nallocCandinfo[*first].end()) {
                        continue;//もうナロックではないから
                    }
                }

                if (item.second == true) {
                    d->graph[*first].core[item.first].candFlg = true;
                    newInitCMap[*first].push_back(item.first);
                }
            }
        }

        for (const auto& item : d->enableinfo[*first]) {
            if (item.second == true) {
                d->graph[*first].core[item.first].enable = true;
                newInitEMap[*first].push_back(item.first);
            }
            else {
                d->graph[*first].core[item.first].enable = false;
                d->graph[*first].core[item.first].rate[d->graph[*first].adjNode[0]] = 0;
                d->graph[*first].core[item.first].rate[d->graph[*first].adjNode[1]] = 0;
                d->graph[*first].core[item.first].delay[d->graph[*first].adjNode[0]] = d->graph[*first].propagation;
                d->graph[*first].core[item.first].delay[d->graph[*first].adjNode[1]] = d->graph[*first].propagation;
                d->graph[*first].core[item.first].useAppRate.clear();
            }
        }
    }

    d->initCandInfoMap = newInitCMap;
    d->initEnableInfoMap = newInitEMap;//初期値を更新


}


