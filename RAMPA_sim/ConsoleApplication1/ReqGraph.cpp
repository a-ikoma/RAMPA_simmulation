#include "ReqGraph.h"

using namespace std;

ReqGraph::ReqGraph(int id) {
    App_id = id;
    linkCount = 0;
}

ReqGraph::ReqGraph() {

}

vector<string> split_naive2(const string& s, char delim) {
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

ReqGraph::ReqMap ReqGraph::createGraph(string reqGraph) {
    ReqGraph::ReqMap map;

    map[boost::graph_bundle].id = App_id;
    int count = 0;
    linkCount = 0;
    string data;



    vector<string> graphdata = split_naive2(reqGraph, '#');
    for (int t = 0; t < graphdata.size(); t++) {
        data = graphdata[t];
        if (data.find("cpu") != std::string::npos) {//演算リソースのノード
            vector<string> calc = split_naive2(data, ',');
            ReqMap::vertex_descriptor v = add_vertex(map);
            map[v].number = atoi(calc[1].c_str());
            map[v].resource = 2;
            vector<string> proc = split_naive2(calc[2], '-');
            vector<string> type = split_naive2(calc[3], '-');
            for (int i = 0; i < proc.size(); i++) {
                map[v].type[atoi(proc[i].c_str())] = atoi(type[i].c_str());
            }
        }
        else if (data.find("gpu") != std::string::npos) {//演算リソースのノード
            vector<string> calc = split_naive2(data, ',');
            ReqMap::vertex_descriptor v = add_vertex(map);
            map[v].number = atoi(calc[1].c_str());
            map[v].resource = 3;
            vector<string> proc = split_naive2(calc[2], '-');
            vector<string> type = split_naive2(calc[3], '-');
            for (int i = 0; i < proc.size(); i++) {
                map[v].type[atoi(proc[i].c_str())] = atoi(type[i].c_str());
            }
        }
        else if (data.find("mem") != std::string::npos) {//リモートメモリのノード

            vector<string> calc = split_naive2(data, ',');
            ReqMap::vertex_descriptor v = add_vertex(map);
            map[v].number = atoi(calc[1].c_str());
            map[v].resource = 1;
            vector<string> proc = split_naive2(calc[2], '-');
            vector<string> type = split_naive2(calc[3], '-');
            for (int i = 0; i < proc.size(); i++) {
                map[v].type[atoi(proc[i].c_str())] = atoi(type[i].c_str());
            }

        }
        else if (data.find("link") != std::string::npos) {//リンクの要素定義
            vector<string> link = split_naive2(data, ',');
            auto vertex_range = vertices(map);
            bool inserted = true;
            bool flg = false;
            ReqMap::edge_descriptor e;
            int pairNode = -1;
            for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
                if (flg == false && (map[*first].number == atoi(link[1].c_str()) || map[*first].number == atoi(link[2].c_str()))) {
                    pairNode = map[*first].number;
                    flg = true;
                }
                else if (pairNode != map[*first].number && flg == true && (map[*first].number == atoi(link[1].c_str()) || map[*first].number == atoi(link[2].c_str()))) {
                    boost::tie(e, inserted) = add_edge(map[*first].number, pairNode, map);
                    map[*first].adjNode.push_back(pairNode);
                    map[pairNode].adjNode.push_back(map[*first].number);
                    map[e].adjNode.push_back(map[*first].number);
                    map[e].adjNode.push_back(pairNode);
                    map[e].number = linkCount;
                    map[e].write_rate =  stod(link[3].c_str());
                    map[e].read_rate = stod(link[4].c_str());
                    vector<string> proc = split_naive2(link[5], '-');
                    linkCount++;
                    for (int i = 0; i < proc.size(); i++) {
                        map[e].process.push_back(atoi(proc[i].c_str()));

                        //elems.push_back(item);
                    }
                    break;
                }
            }
        }
    }

    return map;
}
