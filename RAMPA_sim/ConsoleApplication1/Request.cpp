#include "Request.h"
#include <boost/graph/depth_first_search.hpp>
using namespace std;
Request::Request(std::string filename,int id) {
    gpuFlg = false;
    ocpyFlg = false;

	file = filename;
    createGraph();


    task_proc = createTask();
    createLinkTask();
    reqID = id;
    allMem = 0;
    allCPU = 0;
    allGPU = 0;



    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {

        if (graph[*first].resource == 1) {
            allMem += 1;
        }
        else if (graph[*first].resource == 2) {
            allCPU += 1;
        }
        else if (graph[*first].resource == 3) {
            allGPU += 1;
        }
    }


}

Request::Request(ModelData* m,int rNum){
    model = m;
    reqID = rNum;
}

std::map<int, std::vector<int>> Request::getTask()
{
    return task_proc;
}

int Request::getTaskNum()
{
    return task_num;
}

int Request::getHopLimit()
{
    return hopLimit;
}



vector<string> split_naive3(const string& s, char delim) {
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

void Request::createGraph(){//要求グラフを作成
    string data;
    map<string, vector<string>> proc_node;
    std::ifstream ifs(file);
    int id;
    if (!ifs) {
        std::cout << "ファイルが開けませんでした。" << std::endl;
        std::cin.get();
        ReqGraph::ReqMap a;
        return ;
    }

    std::string buf;
    string graphText;
    //cout << "要求テキスト：\n" << graphText<<"\n";
    int count = 0;
    id = -1;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        data = buf;
        if (data.find("NODE") != std::string::npos) {
            vector<string> node = split_naive3(data, ',');
            for (int i = 0; i < atoi(node[4].c_str()); i++) {
                graphText += node[3] + "," + std::to_string(count) +","+node[1] + "," + node[2] + "#";
                vector<string> proc = split_naive3(node[1], '-');
                vector<string> type = split_naive3(node[2], '-');
                for (int j = 0; j < proc.size(); j++) {
                    if (type[j] == "0") {
                        proc_node[proc[j] + "_recv"].push_back(std::to_string(count));
                    }else {
                        proc_node[proc[j] + "_exe"].push_back(std::to_string(count));
                    }
                }
                count++;
            }
        }else if (data.find("PROC") != std::string::npos) {
            vector<string> proc = split_naive3(data, ',');
            int cnt = 0;
            for (int i = 0; i < proc_node[proc[1] + "_recv"].size();i++) {
                for (int j = 0; j < proc_node[proc[1] + "_exe"].size(); j++) {
                    if (atoi(proc_node[proc[1] + "_recv"][i].c_str()) >= 2) {//プロセス3だけすべてをつけない
                        if (j == cnt) {
                            graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + "," + proc[2] + "," + proc[3] + "," + proc[1] + "#";
                            cnt++;
                            break;
                        }

                    }
                    else {
                        graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + "," + proc[2] + "," + proc[3] + "," + proc[1] + "#";

                    }
                    //graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + "," + proc[2] + "," + proc[3] + "," + proc[1] + "#";
                    //graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + "," + proc[2] + "," + proc[3] + "," + proc[1] + "#";
                    //graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + "\n";
                }
            }
            proc_clock[atoi(proc[1].c_str())] = stod(proc[4].c_str());
            proc_pageFault[atoi(proc[1].c_str())] = stod(proc[5].c_str());
            proc_pageNum[atoi(proc[1].c_str())] = stod(proc[6].c_str());
        }else if (data.find("APPID") != std::string::npos) {
            vector<string> APPid = split_naive3(data, ',');
            id = atoi(APPid[1].c_str());
        }else if (data.find("PARAM") != std::string::npos) {
            vector<string> paramator = split_naive3(data, ',');
            param = stod(paramator[1].c_str());
        }else if (data.find("HOP") != std::string::npos) {
            vector<string> paramator = split_naive3(data, ',');
            hopLimit = atoi(paramator[1].c_str());
        }
    }
    //cout << graphText;
    ReqGraph reqGraph = ReqGraph(id);
	graph = reqGraph.createGraph(graphText);
    linkCount = reqGraph.linkCount;
    return;
}

std::map<int, std::vector<int>> Request::createTask(){
    string data;
    std::ifstream ifs(file);
    map<int, std::vector<int>> task;
    int count=0;
    if (!ifs) {
        std::cout << "ファイルが開けませんでした。" << std::endl;
        std::cin.get();
        ReqGraph::ReqMap a;
        return std::map<int, std::vector<int>>();
    }
    cout << "File :" << file << "\n";
    std::string buf;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        data = buf;
        if (data.find("TASK") != std::string::npos) {
            count++;
            vector<string> task_gen = split_naive3(data, ',');
            vector<string> task_proc2 = split_naive3(task_gen[2], '-');
            for (int i = 0; i < task_proc2.size(); i++) {
                task[atoi(task_gen[1].c_str())].push_back(atoi(task_proc2[i].c_str()));
            }
            acceptableTime = stod(task_gen[3].c_str());//制限時間を設定
            cout << task_gen[3] << "\n";
            tasks.push_back(atoi(task_gen[1].c_str()));
        }
    }
    task_num = count;

    return task;
}

int vector_finder(std::vector<int> vec, int number) {
    auto itr = std::find(vec.begin(), vec.end(), number);
    size_t index = std::distance(vec.begin(), itr);
    if (index != vec.size()) { // 発見できたとき
        return 1;
    }
    else { // 発見できなかったとき
        return 0;
    }
}





void Request::createLinkTask() {
    for (int i = 0; i < task_num; i++) {
        //task_proc[i+1][j]で各プロセスにアクセス
        std::vector<std::pair<int, int>> linkSet;
        auto edge_range = edges(graph);
        int count = 0;
        for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
            if (vector_finder(graph[*first].process, task_proc[i + 1][0])) {//プロセス担当する仮想リンクがあれば
                std::vector<std::pair<int, int>> tmp = linkSet;
                tmp.push_back(std::make_pair(graph[*first].adjNode[0], graph[*first].adjNode[1]));
                if (0 == task_proc[i + 1].size() - 1) {//全てOKだから
                    link_task[i + 1].push_back(tmp);
                    //graph[*first].nextProcEdges.push_back()
                }
                else {
                    int nodeNum1 = graph[*first].adjNode[0];
                    int nodeNum2 = graph[*first].adjNode[1];
                    //graph[*first].nextProcEdges.push_back(std::make_pair(nodeNum1, nodeNum2));
                    search(i + 1, 1, tmp, nodeNum1, nodeNum2);
                }

            }
        }
    }
    return;
}



void Request::search(int taskNum, int seq, std::vector<std::pair<int, int>> linkSet, int n1, int n2) {//seqはプロセスナンバー
    //task_proc[taskNum][seq]でプロセス番号にアクセス
    ReqGraph::ReqMap::edge_descriptor preE = edge(n1, n2, graph).first;

    auto edge_range = edges(graph);
    int count = 0;
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        int s = graph[*first].adjNode[0];
        int t = graph[*first].adjNode[1];
        if (vector_finder(graph[*first].process, task_proc[taskNum][seq]) && (s == n1 || s == n2 || t == n1 || t == n2)) {//プロセス担当する仮想リンクがあればかつ前の仮想リンクとつながっているリンクだったら
            std::vector<std::pair<int, int>> tmp = linkSet;
            tmp.push_back(std::make_pair(s, t));
            graph[preE].nextProcEdges.push_back(std::make_pair(s, t));
            if (seq == task_proc[taskNum].size() - 1) {//全てOKだから
                link_task[taskNum].push_back(tmp);
            }else {
                search(taskNum, seq + 1, tmp, s, t);
            }
        }
    }
}





void Request::makeRequestFromStages(int stages) {//ステージ数ごとのリクエスト作成
    //まず、ステージ数ごとのRequestの生成文字列を作成
    /*
        APPID, 1
        NODE, 1, 1, gpu, 1
        NODE, 1 - 2, 0 - 1, gpu, 1
        NODE, 2 - 3, 0 - 1, gpu, 1
        PROC, 1, 0, 0, 0, 1, 1
        PROC, 2, 0, 0, 0, 1, 1
        PROC, 3, 0, 0, 0, 1, 1
        TASK, 1, 1 - 2 - 3, 40000
        PARAM, 1
        HOP, 8
    */
    map<string, vector<string>> proc_node;
    int count = 0;

    stageNum = stages;

    string graphText;
    for (int i = 1; i <= stages; i++) {//"NODE"に対応
        vector<string> proc;
        vector<string> type;
        if (i == 1) {
            graphText += "gpu," + std::to_string(count) + ",1,1#";
            proc = { "1" };
            type = { "1" };
        }else {
            graphText += "gpu," + std::to_string(count) + "," + std::to_string(i-1) + "-"+ std::to_string(i) + ",0-1" + "#";

            proc = { std::to_string(i - 1),std::to_string(i) };
            type = { "0","1"};
        }
        for (int j = 0; j < proc.size(); j++) {
            if (type[j] == "0") {
                proc_node[proc[j] + "_recv"].push_back(std::to_string(count));
            }
            else {
                proc_node[proc[j] + "_exe"].push_back(std::to_string(count));
            }
        }
        count++;
    }


    for (int t = 1; t <= stages; t++) {//"PROC"に対応
        vector<string> proc = {"PROC",std::to_string(t)};
        int cnt = 0;
        for (int i = 0; i < proc_node[proc[1] + "_recv"].size(); i++) {
            for (int j = 0; j < proc_node[proc[1] + "_exe"].size(); j++) {
                if (atoi(proc_node[proc[1] + "_recv"][i].c_str()) >= 2) {//プロセス3だけすべてをつけない
                    if (j == cnt) {
                        graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + ",0,0," + proc[1] + "#";
                        cnt++;
                        break;
                    }

                }
                else {
                    graphText += "link," + proc_node[proc[1] + "_recv"][i] + "," + proc_node[proc[1] + "_exe"][j] + ",0,0," + proc[1] + "#";

                }
            }
        }
        proc_clock[t] = 0;
        proc_pageFault[t] = 1;
        proc_pageNum[t] = 1;

    }


    param = 1;
    hopLimit =8;

    ReqGraph reqGraph = ReqGraph(reqID);
    graph = reqGraph.createGraph(graphText);
    linkCount = reqGraph.linkCount;



    //createTASKの処理
    map<int, std::vector<int>> task;
    for (int i = 1; i <= stages; i++) {//"PROC"に対応
        task[1].push_back(i);
    }
    acceptableTime = model->acceptableTime;//制限時間を設定
    throughPut = model->throughPut;//スループット制約を設定
    tasks.clear();
    tasks.push_back(1);
    task_num = 1;

    task_proc = task;


    link_task.clear();
    createLinkTask();

    allMem = 0;
    allCPU = 0;
    allGPU = 0;



    auto vertex_range = vertices(graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {

        if (graph[*first].resource == 1) {
            allMem += 1;
        }
        else if (graph[*first].resource == 2) {
            allCPU += 1;
        }
        else if (graph[*first].resource == 3) {
            allGPU += 1;
        }
    }

}