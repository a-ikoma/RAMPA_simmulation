#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include "ResourceAnt.h"
#include <future>
#include "AllocateData.h"
//#include <boost/thread.hpp>
class AntColony
{
    
private:
    std::map<int, ResourceAnt> bestAnts;

    int antSedai;//�a�̐��㐔
    int antNum;//�P����̋a�̐�
    double feromonGensui;//�t�F�������̌�����
    double feromonParam;//�t�F�������̏d�v�x�p�����[�^
    double costParam;//�R�X�g�̏d�v�x�p�����[�^
    double exeTimeParam;//�\�����s���Ԃ̕��U�ւ̃p�����[�^
    AllocateData* allocData;
    int threads;
    bool hikakuFlg;
    double feromonRate;

public:
    AntColony();
    void threadAnt(int ants, int num, DDCGraph* d, Request* req, double feroP, double costP, int curSedai);
    AntColony(AllocateData* alloc, int sedai, int antnum, double gensui, double feroP, double costP, int threadNumber, double feroRate);
    bool procEmb(DDCGraph* d, Request* req);
    ResourceAnt bestEmb;
    std::map<DDCGraph::Map::edge_descriptor, double> edgeCosts;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> tmpDelay;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> nallocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> allocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> enableinfo;
    std::map< DDCGraph::Map::edge_descriptor, std::map < int, std::vector<std::pair<std::map<int, double>,int>>>> useAppRates;
    //�����N�ԍ��ƃR�A�ԍ��ƃ\�[�X�m�[�h�Łi���[�g�ƒx���j->map[�����N�ԍ�][map<>]
    std::map<int, std::map<int, std::map<int, double[2]>>> rate_delay;
    std::map<int, std::vector<std::pair<std::pair<int, int>, int>>> EdgeMap;
    std::pair<std::vector<int>, std::pair<double, double>> partitionSol;//���f���������@�̉�
    void updateLinkAntInfo();
    double bestCost;
    void updateNewGraph(DDCGraph* d, Request* req);
    std::vector<bool> results;
    std::map<int, int> resourceMap;
    std::map<int, int> residualData;//���\�[�X�̎c�]��(�m�[�h�ԍ�,�c�]�̈�)
    //sstd::map<int, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;//�T���i�K�ɂ����鉼�̌��p�X
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;
    int relatedAppNum;

    int syuusokuNum;

    double allocationTime;

    std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>> HukanouLightPath;


    std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*> edgemaps;//�A�v�����Ƃ̃G�b�W�}�b�v
    std::vector<Request*> reqs;
    std::vector<std::map<int, int>*> resourcemaps;
    int optStage;//�œK���̃X�e�[�W��

    bool curFoundFlg;//���݉����������Ă邩�ǂ���


};

