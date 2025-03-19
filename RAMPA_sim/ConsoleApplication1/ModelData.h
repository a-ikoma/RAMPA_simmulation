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


    int ID;//���f����ID

    int opeNum;//�I�y���[�V�����̐�

    std::vector<double> flopNum; //�e�I�y���[�V������FLOPs
    std::vector<double> memConsume; //�e�I�y���[�V�����̕K�v������
    std::vector<double> outDataSize;//�e�I�y���[�V�����̏o�̓f�[�^�T�C�Y
    double acceptableTime;//�������ԗv��

    double throughPut;//�X���|�v�b�g�v��

    int defineStage;//��r��@�ŕ����������肳��Ă���Ƃ��́A���炩���ߌ��肳���X�e�[�W��



};

