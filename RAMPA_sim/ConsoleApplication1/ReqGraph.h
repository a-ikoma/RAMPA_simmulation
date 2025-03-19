#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

class ReqGraph{//�v���O���t�̍쐬�p
        // variable
private:


    // accessor
public:
    int App_id;
    struct node {
        int number;
        int resource;//1��������,2�͉��Z���\�[�X,3��GPU
        std::map<int, int> type;//(�v���Z�X�ԍ��Ǝ�M�����M��)��M1���M(�����[�g�������ɑ����͂�)0
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double write_rate;//�������ݏ����̃p�P�b�g������
        double read_rate;//�ǂݍ��ݏ����̃p�P�b�g������
        std::vector<int> process;//���̗v�������N�̒S���v���Z�X
        std::vector<int> adjNode;
        std::vector<std::pair<int, int>> nextProcEdges;//���̃v���Z�X�̃G�b�W����ێ�������
    };

    struct bundle {
        int id;//�A�v����ID
    };






    typedef boost::adjacency_list<
        boost::listS, boost::vecS, boost::undirectedS,
        node,    // ���_��Bundle�v���p�e�B
        link, // �ӂ�Bundle�v���p�e�B
        bundle  // �O���t��Bundle�v���p�e�B
    > ReqMap;

    ReqGraph(int id);

    ReqGraph();

    ReqGraph::ReqMap createGraph(std::string reqGraph);



    int linkCount;

};

