#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <stdio.h>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>
#include <queue>
#include <boost/graph/depth_first_search.hpp>
#include <omp.h>


class DDCGraph//��ՂƂȂ�}�C�N���f�[�^�Z���^�[�l�b�g���[�N���쐬����N���X
{

public:

    struct coreInfo {
        bool enable;//���s���ɗ��p�����R�A���ǂ���
        bool candFlg;//�I�����ɂȂ邩�ǂ����̃t���O
        int coreNum;
        std::map<int, double> rate;//�p�P�b�g������
        double feromon;//�t�F������
        std::set<int> intCores;//���������N�̏W��
        std::vector<std::pair<std::map<int, double>,int>> useAppRate;//�A�v�����Ƃ̃p�P�b�g���������L�^�������(���蓖�ĉ��������̎��ɗ��p����)
        std::map<int, double> delay;//�����x���{�`�d�x��
    };
    // variable
private:
    std::string file;


    struct node {
        int number;
        int resource;//������1,cpu2,GPU3,OCS4,�p�P�b�g�X�C�b�`5
        int block;//�R�A���܂��̓������̃u���b�N�̑���
        int residual;//�c�]�u���b�N��
        int id;//���\�[�X�̐��\���Ƃ�id
        double trans_delay;//�m�[�h�̓]�������x��
        double cost;//�R�X�g
        double flops;//FLOPS
        double feromon;//�t�F������
        double capacity;//VRAM�e��
        double memBand;//�������ш敝
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double propagation;//�`�d�x��
        double cost;//�����N�̃R�X�g
        std::unordered_map<int,coreInfo> core;//�n�b�V���͂������̒ʂ��
        std::vector<int> adjNode;
        int passWeight;//�ŏ��z�b�v�T���̂��߂����̒l
        int coreNumber;//�m�[�h�Ԃ̃����N��
    };

    struct bundle {
        std::string name;
    };

    
    
    // accessor
public:
    int allocatePolicy;
    double decreaseRate;//�t�F�������̌�����
    typedef boost::adjacency_list<//multisetS
        boost::listS, boost::vecS, boost::undirectedS,
        node,    // ���_��Bundle�v���p�e�B
        link, // �ӂ�Bundle�v���p�e�B
        bundle  // �O���t��Bundle�v���p�e�B
    > Map;
    DDCGraph(std::string filename,double gensui, double initFeromon,int allocPolicy);
    DDCGraph();

    DDCGraph::Map createGraph();



    double bandwidth;


    double initialFeromon;//�t�F�������̏����l

    std::map<int,std::vector<int>> distances;



    int CPUResidual;
    int GPUResidual;
    int memResidual;

    std::map<DDCGraph::Map::edge_descriptor, std::map<int, std::vector<std::pair<DDCGraph::Map::edge_descriptor, int>>>> lightPaths;//�L�[�Ƃ��ă\�[�X�m�[�h�̔ԍ��A�o�����[�Ƃ��āA�L�[���ŏ��ɒʉ߂���R�A�̔ԍ��ł���A�ʉ߃����N�ƃR�A�ԍ��̃y�A�̏W��
private:

    std::vector<std::pair<int, int>> gpuPairs;
    

    std::vector<Map::vertex_descriptor> cpuIter;
    std::vector<Map::vertex_descriptor> gpuIter;
    std::vector<Map::vertex_descriptor> memIter;
    std::vector<Map::vertex_descriptor> packetIter;
    typedef Map::vertex_descriptor   Vertex;
    void derive_shortest_path(Map::vertex_descriptor s, int edge1, int edge2,
        std::map<int, long long>& dis, std::map<int, long long>& dis2,
        std::map<int, int>& prev,
        std::map<int, int>& count, std::map<int, int>& edgeCount);


    std::map<Map::edge_descriptor,std::map<int,std::map<int, long long>>> shortestHop;//shortest hop between nodes
    std::map < Map::edge_descriptor, std::map<int, std::map<int, long long>>> shortestHopExceptEdge;//shortest hop between nodes (except path through target edge)
    std::map < Map::edge_descriptor, std::map<int, std::map<int, int>>> shortestPaths;//number of shortest paths between nodes
    std::map < Map::edge_descriptor, std::map<int, std::map<int, int>>> shortestPathsExceptEdge;//number of shortest paths between nodes(except path through target edge)

public:
    Map graph;
    double maxGPUCost;//GPU�R�X�g�񂨍ő�l
    int allocCount = 0;
    std::map<Map::edge_descriptor, std::vector<int>> initCandInfoMap;//candFlg��true�̃G�b�W�f�B�X�N���v�^�[�ƃR�A������
    std::map<Map::edge_descriptor, std::vector<int>> initEnableInfoMap;//enableFlg�̏����l�����������

    void decreaseNodeFeromon();
    void resetFeromon();//�t�F�������l�̃��Z�b�g


    void setResidual(std::map<int, int> residualData);
    int calcCoreMapID(std::set<int> cores);
    void updateResourceCost(std::map<Map::vertex_descriptor,double>* resourceCosts, std::map<int, int>* residualData);
    void updateLinkCost(std::map<Map::edge_descriptor, double>* edgeCosts, std::map<Map::vertex_descriptor, double>* resourceCosts,std::map<int, int>* resData);

    std::vector<int> allocateResourceRange;
    void setAllocateResourceRange();
    void eraseLightPath();
    void setAllocCandInfo();
    double propagationValue;
    double propagationValue_resource;
    std::string graphName;


    void deriveNodeDistance(Map::vertex_descriptor s, std::vector<int>& dis);

    double circuitDelay;

    std::vector<Map::edge_descriptor> swLinks;//�X�C�b�`�Ԃ̃����N�̏W��

    int memAllResidual;//�����̎c�]
    int CPUAllResidual;//�����̎c�]
    int GPUAllResidual;//�����̎c�]


    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> nallocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> allocCandinfo;
    std::map<DDCGraph::Map::edge_descriptor, std::map<int, bool>> enableinfo;


    std::vector<int> fixAllocateCandidates;//����GPU�v�[�����̎������炷�ׂđI�Ԏ��̌��



    int allNodeNum;


    std::vector<int> maxResidualNode;

};