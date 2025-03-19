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


	void search(int taskNum, int seq, std::vector<std::pair<int, int>> linkSet, int n1, int n2);

public:
	Request(std::string filename,int id);
	Request(ModelData* m, int rNum);
	int reqID;
	std::map<int, std::vector<int>> getTask();
	int getTaskNum();
	ReqGraph::ReqMap graph;
	std::map<int, std::vector<int>> task_proc;
	std::map<int, std::vector<std::vector<std::pair<int,int>>>> link_task;
	std::map<int, double> proc_clock;
	std::map<int, double> proc_pageFault;
	std::map<int, double> proc_pageNum;
	double param;
	std::vector<int> tasks;
	int getHopLimit();
	bool ocpyFlg;

	int allCPU;
	int allGPU;
	int allMem;
	bool gpuFlg;
	std::vector<int> procGPU;
	bool connectGPUs;
	int linkCount;
	int connectGPUProcess;
	double connectGPURate;


	int hopLimit;

	double acceptableTime;
	double throughPut;
	void makeRequestFromStages(int stages);


	ModelData* model;

	int stageNum;

};

