#include "ModelData.h"


using namespace std;


vector<string> split_modelData(const string& s, char delim) {
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


ModelData::ModelData(std::string filename, int partStage){
    setData(filename);

    defineStage = partStage;

}

void ModelData::setData(std::string filename) {


    string data;
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cout << "ファイルが開けませんでした。(ModelData)" << std::endl;
        std::cin.get();
        return;
    }

    std::string buf;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        vector<string> data = split_modelData(buf, ',');
        if (buf.find("ID") != std::string::npos) {
            ID= atoi(data[1].c_str());
        }
        else if (buf.find("OPE") != std::string::npos) {
            opeNum = atoi(data[1].c_str());
        }
        else if (buf.find("FLOPs") != std::string::npos) {
            for (int i = 1; i < opeNum+1;i++) {
                flopNum.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("MEM") != std::string::npos) {
            for (int i = 1; i < opeNum + 1; i++) {
                memConsume.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("OUT") != std::string::npos) {
            for (int i = 1; i < opeNum + 1; i++) {
                outDataSize.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("TIME") != std::string::npos) {
            acceptableTime = stod(data[1].c_str());
        }
        else if (buf.find("THROUGH") != std::string::npos) {
            throughPut = stod(data[1].c_str());
        }

    }
}

