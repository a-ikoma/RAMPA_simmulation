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
    int ID;
    int opeNum;
    std::vector<double> flopNum;
    std::vector<double> memConsume;
    std::vector<double> outDataSize;
    double acceptableTime;
    double throughPut;
    int defineStage;
};

