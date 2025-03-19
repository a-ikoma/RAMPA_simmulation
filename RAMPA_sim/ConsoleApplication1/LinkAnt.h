#pragma once
#include "DDCGraph.h"
#include "Request.h"
#include <map>
#include <random>


class LinkAnt{//�����N�̖��ߍ��ݏꏊ��T������A��
private:


	//���z�����N�ƕ����G�b�W�Ƃ��̃R�A�̑g���}�b�s���O
	bool missFlg = false;//�������s���ǂ���
	
	double feromonParam;//�t�F�������̏d�v�x�p�����[�^
	double costParam;//�R�X�g�̏d�v�x�p�����[�^




	//��r��@���ǂ����i�\�����Ԃ�Z������j
	void initCandInfo(DDCGraph* ddcGraph, Request* req);
	void embIntLink(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor, int coreNum,int source,int target, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateCandInfo(DDCGraph* ddcGraph, Request* req, DDCGraph::Map::edge_descriptor e, int coreNum, int source, ReqGraph::ReqMap::edge_descriptor vedge);
	void updateRate(DDCGraph* ddcGraph, Request* req);
	double baseFeromon;//���Ƃ��đI�΂�Ă��Ȃ������N�̃t�F�������̒l
	//std::map<DDCGraph::Map::edge_descriptor, double>* edgeCosts;
	std::vector<int> distance;
	std::vector<int> tarDistance;
	double procMinTime;//�e�v���Z�X�̍ŏ����Ԃ�ۑ�
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> tmpRate;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int, double>>> checkDelay;//�e�����N�̒x���f�[�^
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
	std::map<int, std::vector<std::pair<std::pair<int,int>, int>>> EdgeMap;//<�����N�̗אڃm�[�h�̑g�ƃR�A�ԍ��̑g>
	double allCost;
	//std::map<int, std::map<int,double>>* feromonInfo;//�R�A���̎Q��
	std::map<DDCGraph::Map::edge_descriptor, std::map<int,bool>>* nallocCandinfo;
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* allocCandinfo;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> linkant_newAllocatedLink;//�����N�A���g�ŐV�������蓖�Ă�ꂽ���
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> newAllocCandinfo;//�V�������蓖�Ă�ꂽ��̏W��

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>>* enableinfo;
	//�����N�ԍ��ƃR�A�ԍ��ƃ\�[�X�m�[�h�Łi���[�g�ƒx���j->map[�����N�ԍ�][map<>]
	std::map<int, std::map<int, std::map<int, double[2]>>> rate_delay;

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::map<int,double>>> tmpDelay;//��r��@�p�̒x���ێ��}�b�v
	//void updateNewGraph();
	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>>* tmp_lightPaths;//�T���i�K�ɂ����鉼�̌��p�X

	std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> newlightPaths;//�T���i�K�ɂ�����V�K�̌��p�X

	double feromonValue(std::map<DDCGraph::Map::edge_descriptor, std::map<int, double>>* settingFeromons,
		DDCGraph::Map::edge_descriptor e, int coreNum);

	bool checkOcupy(DDCGraph* d, Request* req, DDCGraph::Map::edge_descriptor e, int core);

	void checkAllocApp(DDCGraph* d, DDCGraph::Map::edge_descriptor e, int core);


	int hopLimit;//�z�b�v����
	double predictTime;//��r��@�p�̎���

	std::map < DDCGraph::Map::edge_descriptor, std::map < int, std::pair<std::map<int, double>, int>>> newAppRate;//���̃A�v���̖��ߍ��݂ōs��s������郌�[�g�ʂ�ۑ�

	std::map< DDCGraph::Map::edge_descriptor, std::map < int,std::vector<std::pair<std::map<int,double>,int>>>> useAppRates;

	std::vector //�e�A�v��
	< std::vector //�e�v���Z�X��
	<std::vector//�e�v���Z�X
	< std::pair<std::pair<double, double>,//�y�[�W�t�H�[���g��,���Z���\�[�X�Ƃ��̎���
	std::vector<std::pair< std::pair<DDCGraph::Map::edge_descriptor, int>, int >
	>>>>>* appEdges;
	std::vector<double>* otherAppLimits;//�y�[�W�t�H�[���g���Ɛ�������


	std::vector<std::map<int, std::vector<std::vector<std::pair<int, int>>>>> linktasks;//�A�v�����Ƃ̃^�X�N�̃����N
	std::vector<double> limits;
	double baseCost;//�}�؂�̂���


	std::vector<std::map<int, std::vector<std::pair<std::pair<int, int>, int>>>*>* edgemaps;//�A�v�����Ƃ̃G�b�W�}�b�v
	std::vector<Request*>* reqs;
	std::vector<std::map<int, int>*>* resourcemaps;

	std::set<int> appIDs;
	double maxTraffic[4];

	double allocatedResourceCost;


	DDCGraph::Map::edge_descriptor curLightPathSourceEdge;//���ݓ˓����Ă�����p�X�̍ŏ��ɓ��郊���N
	int curLightPathSourceCore;//���ݓ˓����Ă�����p�X�̍ŏ��ɓ��郊���N�̃R�A�ԍ�

	std::map<int, int> resourceMap;
	double curLightPathCost;//���݂̌��p�X�̃R�X�g



	std::map<int, std::map<int, double>> resourcePairPropagation;//�\�[�X�m�[�h����^�[�Q�b�g�m�[�h�܂ł̓`���x���{�X�C�b�`�����x���̑��a
	double currentResourcePairPropagation;//���T�����̎����Ԃ̓`���x��

	std::pair<std::vector<int>, std::pair<double, double>> partitionSol;//���f���������@�̉�

};