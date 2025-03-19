#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

class ReqGraph{
private:

public:
    int App_id;
    struct node {
        int number;
        int resource;
        std::map<int, int> type;
        std::vector<int> adjNode;
    };

    struct link {
        int number;
        double write_rate;
        double read_rate;
        std::vector<int> process;
        std::vector<int> adjNode;
        std::vector<std::pair<int, int>> nextProcEdges;
    };

    struct bundle {
        int id;
    };






    typedef boost::adjacency_list<
        boost::listS, boost::vecS, boost::undirectedS,
        node,
        link,
        bundle
    > ReqMap;

    ReqGraph(int id);

    ReqGraph();

    ReqGraph::ReqMap createGraph(std::string reqGraph);



    int linkCount;

};

