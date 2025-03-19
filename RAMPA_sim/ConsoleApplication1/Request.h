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


	//std::map<int, vector<std::pair<std::pair<int, int>, int>>>;//仮想リンク番号で順番に通る

	void search(int taskNum, int seq, std::vector<std::pair<int, int>> linkSet, int n1, int n2);







public:
	Request(std::string filename,int id);
	Request(ModelData* m, int rNum);
	int reqID;
	std::map<int, std::vector<int>> getTask();
	int getTaskNum();
	ReqGraph::ReqMap graph;
	std::map<int, std::vector<int>> task_proc;
	std::map<int, std::vector<std::vector<std::pair<int,int>>>> link_task;//タスクを行うリンクの集合
	std::map<int, double> proc_clock;//プロセスごとのクロック数
	std::map<int, double> proc_pageFault;//プロセスごとのページフォールト数
	std::map<int, double> proc_pageNum;//プロセスごとのページ数
	double param;//パラメータ
	std::vector<int> tasks;
	int getHopLimit();
	bool ocpyFlg;
	//std::map<int, std::vector<int>> passedNode;//仮想リンク番号で通るノード番号を順番に

	int allCPU;
	int allGPU;
	int allMem;
	bool gpuFlg;
	std::vector<int> procGPU;
	bool connectGPUs;
	int linkCount;
	int connectGPUProcess;
	double connectGPURate;


	int hopLimit;//ホップ数制限

	double acceptableTime;//許容時間
	double throughPut;//スル－プット要件
	void makeRequestFromStages(int stages);


	ModelData* model;

	int stageNum;

};

