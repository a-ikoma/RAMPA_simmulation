#pragma once

#include <string>

#include <fstream>
#include <iostream>
#include <vector>
class ModelData
{
private:


public:
	ModelData(std::string filename, int partStage);
	void setData(std::string filename);


    int ID;//モデルのID

    int opeNum;//オペレーションの数

    std::vector<double> flopNum; //各オペレーションのFLOPs
    std::vector<double> memConsume; //各オペレーションの必要メモリ
    std::vector<double> outDataSize;//各オペレーションの出力データサイズ
    double acceptableTime;//応答時間要件

    double throughPut;//スル－プット要件

    int defineStage;//比較手法で分割数が決定されているときの、あらかじめ決定されるステージ数



};

