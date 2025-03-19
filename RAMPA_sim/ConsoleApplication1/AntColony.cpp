#include "AntColony.h"



using namespace std;

//map<int,ResourceAnt> bestAnts;
void AntColony::threadAnt(int ants, int num, DDCGraph* d, Request* req, double feroP, double costP, int curSedai) {

    ResourceAnt bestAnt;

    double minCost = -1;
    bool sucessFlg = false;

    for (int i = 0; i < ants; i++) {
        ResourceAnt ant = ResourceAnt(d, antSedai, antNum, feroP, costP, feromonRate,req->getHopLimit(), &reqs, &edgemaps, &resourcemaps, bestCost);
        if (curSedai == 1 && curFoundFlg == false) {//����������Ȃ�������E�E�E
            ant.maxResSearchFlg = true;//<-���������������邩��Ȃ�
        }
        if (ant.ResourceEmbedding(d, req) == true) {//��₾������
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

    antSedai = sedai;//�a�̐��㐔
    antNum = antnum;//�P����̋a�̐�
    feromonGensui = gensui;//�t�F�������̌�����
    feromonParam = feroP;//�t�F�������̏d�v�x�p�����[�^
    costParam = costP;//�R�X�g�̏d�v�x�p�����[�^
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

    //�ŏI�I�Ɋ��蓖�Č���
    bool result = false;
    int endCount = 0;
    for (auto t : req->tasks) {
        cout<<req->acceptableTime<<"\n";
    }
    double preMinCost = 0;

    auto start = std::chrono::system_clock::now(); // �v���J�n����Time:15991

    auto req_range = vertices(d->graph);
    for (auto first = req_range.first, last = req_range.second; first != last; ++first) {//�v���O���t
        residualData[d->graph[*first].number] = d->graph[*first].residual;
    }//�c�]���̏�����
    std::map<DDCGraph::Map::vertex_descriptor, double> tmp;//�����N�̏C�_�̕]����p�����ǁA�����H�Ƃ͈ꗥ������Ƃ肠�����_�~�[�Ƃ��Ă����p��


    for (int t = 0; t < allocData->info.size(); t++) {//�ߋ��̊��蓖�ď��
        edgemaps.push_back(&(allocData->info[t].edgeMap));
        reqs.push_back(allocData->info[t].req);
        resourcemaps.push_back(&(allocData->info[t].resourceMap));
    }

    d->setAllocCandInfo();

    int maxStage = 1;
    int initialStage = 1;

    if (d->allocatePolicy != 0) {//�X�e�[�W�������O����
        maxStage = req->model->defineStage;
        initialStage = req->model->defineStage;
    }else {//
        //�c���GPU���ƃI�y���[�V�����̐��̂����̍ŏ��l���}�b�N�X
        maxStage = min(d->GPUResidual,req->model->opeNum);

        cout << "max:" << maxStage << "\n";
        initialStage = 1;
    }


    for (int stage = initialStage; stage <= maxStage; stage++) {//�e�X�e�[�W�ɑ΂��Ď��������蓖�ĂĂ����܂�
        //�X�e�[�W�����Ƃ̗v������

        cout << "�X�e�[�W���F" << stage<<"\n";
        req->makeRequestFromStages(stage);//���N�G�X�g���č\�z

        double tmpCost = -1;
        bool finFlg = false;
        if (result == true) {//�O�̃X�e�[�W�ŉ���������ꂽ�ꍇ�A���̃X�e�[�W�̒T�����I��莟��I������B�i�R�X�g�I�ɉ��͌�����Ȃ����낤�j
            finFlg = true;
        }


        for (int i = 0; i < antSedai; i++) {////���㐔

            cout << i << "����F";

            int antLoopNum = antNum / threads;


            for (int i = 0; i < results.size(); i++) {
                results[i] = false;
            }

#pragma omp parallel for
            for (int j = 0; j < threads; j++) {
                // ���[�v�ϐ�id�͕K���R�s�[�L���v�`������
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


                if ((bestCost == -1 && tmpCost != -1) || (tmpCost != -1 && bestCost > tmpCost)) {//�p�P�b�g�D��
                    bestCost = tmpCost;
                    updateLinkAntInfo();

                    if (finFlg) {
                        finFlg = false;
                    }
                }

            }

            d->decreaseNodeFeromon();//�t�F����������(1���㕪�̂��ׂĂ����܂�����Ȃ̂Ńt�F�������𒼐ڍX�V�ł���)
            if (result == true) {
                auto req_range = vertices(req->graph);
                set<int> nodeList;
                for (auto vfirst = req_range.first, last = req_range.second; vfirst != last; ++vfirst) {//�v���O���t
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


    auto end = std::chrono::system_clock::now();// �v���I��������ۑ�
    auto dur = end - start;
    allocationTime = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count(); //�����ɗv�������Ԃ��~���b�ɕϊ�
    cout << "������������:" << allocationTime << "\n";





    if (result == true) {
        d->setResidual(residualData);//�c�]�̍X�V



        req->makeRequestFromStages(optStage);//���N�G�X�g���č\�z

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
            //cout << "�m�[�h" << d->graph[*first].number << "(������)" << "�̎c�]��";
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
    for (const auto& item : bestEmb.bestLinkAnt->newAllocCandinfo) {//���蓖�Ă�ꂽ�����N�̃R�A�S�������蓖�Ă��Ȃ��Ȃ�������Ă�Ƃ������
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

    partitionSol= bestEmb.bestLinkAnt->partitionSol;//���f���������@�̉�

    optStage = bestEmb.bestLinkAnt->partitionSol.first.size();//�X�e�[�W��
    relatedAppNum = bestEmb.bestLinkAnt->appIDs.size();
}

void AntColony::updateNewGraph(DDCGraph* d, Request* req) {//�O���t�ɑ΂��Ēx���ƃ��[�g�Aenable�AcandFlg���X�V
    std::map<DDCGraph::Map::edge_descriptor, std::vector<int>> newInitCMap;//candFlg��true�̃G�b�W�f�B�X�N���v�^�[�ƃR�A������
    std::map<DDCGraph::Map::edge_descriptor, std::vector<int>> newInitEMap;//enableFlg�̏����l�����������
    auto edge_range = edges(req->graph);
    DDCGraph::Map::edge_descriptor ed;
    map<DDCGraph::Map::edge_descriptor, map<int, bool>> setFlg;





    int outStageNum = -1;
    double dataPeriod = req->model->throughPut;//�f�[�^���o�͂����Ԋu
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {//�X�V

        double traffic = (req->model->outDataSize[partitionSol.first[req->graph[req->graph[*first].adjNode[1]].number] - 1] / dataPeriod);


        for (auto e : EdgeMap[req->graph[*first].number]) {
          
            ed = edge(e.first.first, e.first.second, d->graph).first;
            if (setFlg[ed][e.second] == true) {
                continue;
            }

            setFlg[ed][e.second] = true;
            d->graph[ed].core[e.second].enable = d->enableinfo[ed][e.second];
            if (d->enableinfo[ed][e.second] == false) {
                cout << "\n\n\n\n�������\������邱�Ƃ���H\n\n\n";
                d->enableinfo[ed][e.second] = true;
            }

            d->graph[ed].core[e.second].rate[e.first.first] = traffic;//�g���q�b�N�͈��Z�ɂ����N���Ȃ�
            d->graph[ed].core[e.second].rate[e.first.second] = 0;//�g���q�b�N�͈��Z�ɂ����N���Ȃ�������A0
            if (d->graph[e.first.first].resource != 5) {//�p�P�b�g����Ȃ�������
                d->graph[ed].core[e.second].delay[e.first.first] = d->graph[ed].propagation + d->graph[e.first.first].trans_delay;
            }else {//��������Ȃ��Ƃ�(��������OCS�݂̂�z�肷��̂ł��̂܂�)
                d->graph[ed].core[e.second].delay[e.first.first] = d->graph[ed].propagation + d->graph[e.first.first].trans_delay;
            }

            if (d->graph[e.first.second].resource != 5) {//�p�P�b�g����Ȃ�������
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
            for (const auto& item : d->allocCandinfo[*first]) {//���Ƃ��炠��A���b�N�L�����h
                if (item.second == true) {
                    d->graph[*first].core[item.first].candFlg = true;
                    newInitCMap[*first].push_back(item.first);

                }
            }
        }

        if (allocCandinfo.find(*first) != allocCandinfo.end()) {
            for (const auto& item : allocCandinfo[*first]) {//�V�K�̃A���b�N�L�����h
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
                        continue;//�����i���b�N�ł͂Ȃ�����
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
    d->initEnableInfoMap = newInitEMap;//�����l���X�V


}


