#pragma once
#include <string>
#include <vector>
#include "ReqGraph.h"
#include <map>
#include "ModelData.h"

class Request{
private:
	std::string file;
	
	int APP_id;
	int task_num;


	void createGraph();
	std::map<int, std::vector<int>> createTask(); 
	void connectingGPUs();
	void createLinkTask();


	//std::map<int, vector<std::pair<std::pair<int, int>, int>>>;//���z�����N�ԍ��ŏ��Ԃɒʂ�

	void search(int taskNum, int seq, std::vector<std::pair<int, int>> linkSet, int n1, int n2);







public:
	Request(std::string filename,int id);
	Request(ModelData* m, int rNum);
	int reqID;
	std::map<int, std::vector<int>> getTask();
	int getTaskNum();
	ReqGraph::ReqMap graph;
	std::map<int, std::vector<int>> task_proc;
	std::map<int, std::vector<std::vector<std::pair<int,int>>>> link_task;//�^�X�N���s�������N�̏W��
	std::map<int, double> proc_clock;//�v���Z�X���Ƃ̃N���b�N��
	std::map<int, double> proc_pageFault;//�v���Z�X���Ƃ̃y�[�W�t�H�[���g��
	std::map<int, double> proc_pageNum;//�v���Z�X���Ƃ̃y�[�W��
	double param;//�p�����[�^
	std::vector<int> tasks;
	int getHopLimit();
	bool ocpyFlg;
	//std::map<int, std::vector<int>> passedNode;//���z�����N�ԍ��Œʂ�m�[�h�ԍ������Ԃ�

	int allCPU;
	int allGPU;
	int allMem;
	bool gpuFlg;
	std::vector<int> procGPU;
	bool connectGPUs;
	int linkCount;
	int connectGPUProcess;
	double connectGPURate;


	int hopLimit;//�z�b�v������

	double acceptableTime;//���e����
	double throughPut;//�X���|�v�b�g�v��
	void makeRequestFromStages(int stages);


	ModelData* model;

	int stageNum;

};

