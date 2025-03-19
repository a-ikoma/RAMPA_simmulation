
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "DDCGraph.h"
#include "ReqGraph.h"
#include "Request.h"
#include "ResourceAnt.h"
#include <thread>
#include <future> // promise/future
#include <utility> // move
#include "AntColony.h"
#include "AllocateData.h"
#include "ModelData.h"

using namespace std;

vector<string> split(const string& s, char delim) {
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

double linkRate(DDCGraph* d) {
    double miwariate = 0;
    double linkNum = 0;
    auto edge_range = edges(d->graph);
    for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
        linkNum += d->graph[*first].coreNumber;
        for (int i = 0; i < d->graph[*first].core.size(); i++) {

            if (d->graph[*first].core[i].enable == true && d->graph[*first].core[i].coreNum == 1) {
                if (d->graph[*first].core[i].rate[d->graph[*first].adjNode[0]] == 0 &&
                    d->graph[*first].core[i].rate[d->graph[*first].adjNode[1]] == 0) {
                    miwariate++;
                }
            }
        }

    }
    return (linkNum - miwariate) / linkNum;
}

pair<float, float> resourceRate(DDCGraph* d) {
    float calcResidual = 0;
    float memResidual = 0;
    auto vertex_range = vertices(d->graph);
    for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {

        if (d->graph[*first].resource == 1) {
            memResidual += d->graph[*first].residual;
        }
        else if (d->graph[*first].resource == 2) {
            calcResidual += d->graph[*first].residual;
        }
    }
    return make_pair(calcResidual, memResidual);
}



int main() {
    string baseDir = "./";

    string data;
    std::ifstream ifs(baseDir+"param.txt");
    if (!ifs) {
        std::cout << "param.txtファイルが開けませんでした。" << std::endl;
        std::cin.get();
        return 0;
    }

    int APPID = -1;

    int modelNumber=-1;

    int antSedai = 0;
    int antNum = 0;
    double feromonGensui = 0.1;
    double feromonParam = 2;
    double costParam = 1;
    int threadNumber = 5;
    double feromonRate = 0.1;
    double initialFeromon = 1;

    int rejectID = -1;


    int allocatePolicy = 0;
    bool logFlg = false;
    bool Tumekomi = false;
    bool allocateOK = false;

    std::vector<int> partition_Stages;

    std::string buf;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        data = buf;
        if (data.find("antSedai") != std::string::npos) {
            antSedai = atoi(split(data, ',')[1].c_str());
        }
        else if (data.find("antNum") != std::string::npos) {
            antNum = atoi(split(data, ',')[1].c_str());
        }
        else if (data.find("feromonGensui") != std::string::npos) {
            feromonGensui = stod(split(data, ',')[1].c_str());
        }
        else if (data.find("feromonParam") != std::string::npos) {
            feromonParam = stod(split(data, ',')[1].c_str());
        }
        else if (data.find("costParam") != std::string::npos) {
            costParam = stod(split(data, ',')[1].c_str());
        }
        else if (data.find("thread") != std::string::npos) {
            threadNumber = atoi(split(data, ',')[1].c_str());
        }
        else if (data.find("feromonRate") != std::string::npos) {
            feromonRate = stod(split(data, ',')[1].c_str());
        }
        else if (data.find("initialFeromon") != std::string::npos) {
            initialFeromon = stod(split(data, ',')[1].c_str());
        }
        else if (data.find("allocatePolicy") != std::string::npos) {
            if (atoi(split(data, ',')[1].c_str()) == 1) {
                allocatePolicy = 1;//EdgePipe
            }
            else if (atoi(split(data, ',')[1].c_str()) == 2) {
                allocatePolicy = 2;//NCAR
            }
            else {
                allocatePolicy = 0;//RAMPA
            }
        }
        else if (data.find("log") != std::string::npos) {
            if (atoi(split(data, ',')[1].c_str()) == 1) {
                logFlg = true;
            }
            else {
                logFlg = false;
            }
        }
        else if (data.find("tumekomi") != std::string::npos) {
            if (atoi(split(data, ',')[1].c_str()) == 1) {
                Tumekomi = true;
            }
            else {
                Tumekomi = false;
            }
        }
        else if (data.find("stage") != std::string::npos) {
            std::vector<string> partitionNumbers = split(data, ',');
            for (int i = 1; i < partitionNumbers.size(); i++) {
                partition_Stages.push_back(atoi(partitionNumbers[i].c_str()));
            }
            
        }
        else if (data.find("models") != std::string::npos) {
            modelNumber = atoi(split(data, ',')[1].c_str());
        }
    }
    cout << modelNumber << "\n";

    std::pair<int,int> lifeTime[2000];
    for (int i = 0; i < 2000; i++) {
        lifeTime[i] = { -1,-1 };
    }

    AllocateData* allocData = new AllocateData();
    int requestNum = 0;
    int rejectNum = 0;

    DDCGraph* d = new DDCGraph(baseDir + "graph.txt", feromonGensui, initialFeromon, allocatePolicy);


    std::ofstream writing_file;
    std::string filename = baseDir + "utility_teian_.csv";
    writing_file.open(filename, std::ios::app);
    writing_file << "タイムスロット,棄却率,リンク利用率,GPU利用率,通信遅延が変化したアプリ数,収束の世代数,割当総コスト,割当時間\n";
    writing_file.close();


    //モデルデータの読み込み
    std::vector<ModelData*> mData;

    for (int i = 0; i < modelNumber; i++) {
        cout<< baseDir + "MODELS/model" << (i + 1);
        mData.push_back(new ModelData(baseDir + "MODELS/model" + std::to_string(i + 1) + ".txt", partition_Stages[i]));
    }



    float cpuResidual = d->CPUResidual;
    float gpuResidual = d->GPUResidual;
    float memResidual = d->memResidual;
    float allCPU = cpuResidual;
    float allGPU = gpuResidual;
    float allMem = memResidual;

    auto edge_range = edges(d->graph);


    int allocId = 0;
    int timeSlot = 0;
    int allocId2 = 0;
    int timeSlot2 = 0;

    int requestCnt=0;

    string seqs;
    std::ifstream ifs2(baseDir + "allocSeq.txt");
    if (!ifs2) {
        std::cout << "param.txtファイルが開けませんでした。" << std::endl;
        std::cin.get();
        return 0;
    }
    std::string linkFile;
    std::ofstream link_writing_file;
    linkFile = baseDir + "link_" + d->graphName + ".csv";


    std::string residualFile;
    std::ofstream residual_writing_file;
    residualFile = baseDir + "residual_" + d->graphName + ".csv";

    std::string lightPathFile;
    std::ofstream lightPath_writing_file;
    lightPathFile = baseDir + "lightPath_" + d->graphName + ".csv";


    int continueReject=0;

    auto vertex_range = vertices(d->graph);

    std::string buf2;
    while (!ifs2.eof()) {
        requestCnt++;

        std::stringstream residualInfo;
        std::stringstream lateLink;
        std::stringstream lpInfo;

        std::getline(ifs2, buf2);
        seqs = buf2;
        if (seqs.empty()) {
            break;
        }
            
        int nextTime = atoi(split(seqs, ',')[0].c_str());
        int nextApp = atoi(split(seqs, ',')[2].c_str());
        int nextAppLifetime = atoi(split(seqs, ',')[3].c_str());
        while (true) {
            timeSlot++;
            if (Tumekomi == false) {
                for (int i = 0; i < 2000; i++) {
                    if (lifeTime[i].first >= 0) lifeTime[i].first++;
                    if (lifeTime[i].first > lifeTime[i].second) {
                        cout << "Release：APP" << allocId << "\n";
                        rejectID = -1;

                        allocData->removeAllocData(i, d);

                        memResidual = 0;
                        cpuResidual = 0;
                        gpuResidual = 0;

                        for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {

                            if (d->graph[*first].resource == 1) {
                                memResidual += d->graph[*first].residual;
                            }
                            else if (d->graph[*first].resource == 2) {
                                cpuResidual += d->graph[*first].residual;
                            }
                            else if (d->graph[*first].resource == 3) {
                                gpuResidual += d->graph[*first].residual;
                            }
                        }
                        d->CPUResidual = cpuResidual;
                        d->GPUResidual = gpuResidual;
                        d->memResidual = memResidual;

                        lifeTime[i] = { -1 ,-1 };
                    }
                }
            }

            if (timeSlot != nextTime) {
                continue;
            }

            cout << "タイムスロット：" << timeSlot << "\n";

            APPID = nextApp;
            cout << "Service" << APPID << "\n";

            AntColony ac;
            bool muriFlg = false;
            Request* req = new Request(mData[APPID-1], requestNum);

            requestNum++;

            if (d->GPUResidual < 1) {
                muriFlg = true;
            }

            allocateOK = false;

            ac = AntColony(allocData, antSedai, antNum, feromonGensui, feromonParam, costParam, threadNumber, feromonRate);
            if ((rejectID == -1 || rejectID != nextApp) && ac.procEmb(d, req) == true) {
                allocateOK = true;
            }


            if (allocateOK) {
                rejectID = -1;
                continueReject = 0;
                lifeTime[allocId] = { 0 ,nextAppLifetime };
                allocData->pushAllocData(allocId, req, ac.resourceMap, ac.EdgeMap, d, ac.partitionSol);
                allocId += 1;
                if (allocId > 2000) allocId = 0;
            }
            else {
                rejectID = nextApp;
                rejectNum += 1;
            }


            d->allocCount += 1;


            memResidual = 0;
            cpuResidual = 0;
            gpuResidual = 0;

            for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {


                if (d->graph[*first].resource == 1) {
                    memResidual += d->graph[*first].residual;
                }
                else if (d->graph[*first].resource == 2) {
                    cpuResidual += d->graph[*first].residual;
                }
                else if (d->graph[*first].resource == 3) {
                    gpuResidual += (float)d->graph[*first].residual;
                }
            }

            

            if (muriFlg == false) {
                writing_file.open(filename, std::ios::app);
                writing_file << timeSlot << "," << req->model->ID << "," <<
                    (double)rejectNum / (double)requestNum << "," << linkRate(d) << "," << (allGPU - gpuResidual) / allGPU << "," <<
                    ac.relatedAppNum << "," << ac.syuusokuNum << "," << ac.bestCost << "," << ac.allocationTime<<"," << ac.partitionSol.second.first << "," << ac.partitionSol.second.second << ",";
                writing_file << "[";
                for (int count : ac.partitionSol.first) {
                    writing_file << count << " ";
                }
                writing_file << "]";
                writing_file << std::endl;
                writing_file.close();
            }

            if (logFlg) {
                auto edge_range = edges(d->graph);
                lateLink << "time," << requestCnt << "\n";
                for (auto first = edge_range.first, last = edge_range.second; first != last; ++first) {
                    lateLink << d->graph[*first].adjNode[0] << "-" << d->graph[*first].adjNode[1] << "-" << d->graph[*first].number << "\n";

                    int count = 0;
                    for (int i = 0; i < d->graph[*first].core.size(); i++) {

                        if (d->graph[*first].core[i].enable == true) {
                            count += d->graph[*first].core[i].coreNum;
                            lateLink << d->graph[*first].core[i].coreNum << "-";
                            lateLink << "(" << d->graph[*first].core[i].rate[d->graph[*first].adjNode[0]] << "," << d->graph[*first].core[i].rate[d->graph[*first].adjNode[1]] << ")";
                            for (auto tmpa : d->graph[*first].core[i].useAppRate) {
                                lateLink << "-" << tmpa.second;
                            }
                            lateLink << "/";
                        }
                    }
                    lateLink << "\n";
                }

                lpInfo << "time," << requestCnt << "\n";
                for (auto tmp : d->lightPaths) {
                    for (auto node : tmp.second) {
                        
                        for (auto edge : node.second) {
                            lpInfo << "(" << d->graph[edge.first].adjNode[0]<<","<< d->graph[edge.first].adjNode[1] << "," << edge.second << ")->";
                        }
                        lpInfo << "\n";
                    }

                }

                residualInfo << "time," << requestCnt << "\n";

                std::map<int, std::pair<int,int>> flops_maxRes;//GPUの種類ごとの残余の最大値

                for (auto first = vertex_range.first, last = vertex_range.second; first != last; ++first) {
                    if (d->graph[*first].resource != 0) {
                        if (d->graph[*first].resource == 1) {
                            residualInfo << "mem-";
                            residualInfo << d->graph[*first].number << "-" << d->graph[*first].residual << ",";
                        }else if (d->graph[*first].resource == 2) {
                            residualInfo << "cpu-";
                            residualInfo << d->graph[*first].number << "-" << d->graph[*first].residual << ",";
                        }else if (d->graph[*first].resource == 3) {
                            residualInfo << "gpu-";
                            residualInfo << d->graph[*first].number << "-" << d->graph[*first].residual << "-" << d->graph[*first].flops << ",";

                            if (flops_maxRes.find(d->graph[*first].flops) != flops_maxRes.end()) {
                                if (flops_maxRes[d->graph[*first].flops].second < d->graph[*first].residual) {
                                    flops_maxRes[d->graph[*first].flops] = { d->graph[*first].number, d->graph[*first].residual };
                                }
                            }
                            else {
                                flops_maxRes[d->graph[*first].flops] = { d->graph[*first].number, d->graph[*first].residual };
                            }
                        }

                    }
                }
                d->maxResidualNode.clear();
                for (auto const& item : flops_maxRes) {
                    d->maxResidualNode.push_back(item.second.first);
                }


                link_writing_file.open(linkFile, std::ios::app);
                link_writing_file << lateLink.str();
                link_writing_file.close();
                residual_writing_file.open(residualFile, std::ios::app);
                residual_writing_file << residualInfo.str();
                residual_writing_file << "\n";
                residual_writing_file.close();
                lightPath_writing_file.open(lightPathFile, std::ios::app);
                lightPath_writing_file << lpInfo.str();
                lightPath_writing_file << "\n";
                lightPath_writing_file.close();
            }
            break;

        }
        if (allocateOK == false) {
            break;
        }
    }
    delete allocData;
    delete d;
    cout << "Done.\n";


    return 0;
}