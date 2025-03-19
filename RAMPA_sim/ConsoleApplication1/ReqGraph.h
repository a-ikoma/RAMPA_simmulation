#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

class ReqGraph{//要求グラフの作成用
        // variable
private:


    // accessor
public:
    int App_id;
    struct node {
        int number;
        int resource;//1がメモリ,2は演算リソース,3がGPU
        std::map<int, int> type;//(プロセス番号と受信か送信か)受信1送信(リモートメモリに多いはず)0
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double write_rate;//書き込み処理のパケット到着率
        double read_rate;//読み込み処理のパケット到着率
        std::vector<int> process;//その要求リンクの担当プロセス
        std::vector<int> adjNode;
        std::vector<std::pair<int, int>> nextProcEdges;//次のプロセスのエッジ情報を保持するやつ
    };

    struct bundle {
        int id;//アプリのID
    };






    typedef boost::adjacency_list<
        boost::listS, boost::vecS, boost::undirectedS,
        node,    // 頂点のBundleプロパティ
        link, // 辺のBundleプロパティ
        bundle  // グラフのBundleプロパティ
    > ReqMap;

    ReqGraph(int id);

    ReqGraph();

    ReqGraph::ReqMap createGraph(std::string reqGraph);



    int linkCount;

};

