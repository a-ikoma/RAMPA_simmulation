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
    /*
ID,1
OPE,4
FLOPs,10,20,30,40
MEM,2,2,2,2
OUT,5,5,5,5
TIME,40000
THROUGH,10
*/
    std::string buf;
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        vector<string> data = split_modelData(buf, ',');//[0]:行、[1]：列
        if (buf.find("ID") != std::string::npos) {//モデルのID
            ID= atoi(data[1].c_str());
        }
        else if (buf.find("OPE") != std::string::npos) {//オペレーションの数
            opeNum = atoi(data[1].c_str());
        }
        else if (buf.find("FLOPs") != std::string::npos) {//各オペレーションのFLOPs
            for (int i = 1; i < opeNum+1;i++) {
                flopNum.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("MEM") != std::string::npos) {//各オペレーションの必要メモリ
            for (int i = 1; i < opeNum + 1; i++) {
                memConsume.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("OUT") != std::string::npos) {//各オペレーションの出力データサイズ
            for (int i = 1; i < opeNum + 1; i++) {
                outDataSize.push_back(stod(data[i].c_str()));
            }
        }
        else if (buf.find("TIME") != std::string::npos) {//応答時間要件
            acceptableTime = stod(data[1].c_str());
        }
        else if (buf.find("THROUGH") != std::string::npos) {//スループット要件
            throughPut = stod(data[1].c_str());
        }

    }
}

