#pragma once
#include "LinkAnt.h"
#include "DDCGraph.h"
#include "Request.h"
class ModelPartition
{
public:
	ModelPartition();
	bool ModelPartitioning(LinkAnt* ant, DDCGraph* ddcGraph, Request* req);
	bool check_capacity(int start, int end, int capacity, LinkAnt* ant, DDCGraph* ddcGraph, Request* req);
	std::pair<bool, std::pair<double, double>> check_total_speed(const std::vector<int>& distribution, LinkAnt* ant, DDCGraph* ddcGraph, Request* req);
	void backtrack(int n, int k, int start, std::vector<int>& current_distribution, int remaining_stages,
		std::vector<std::pair<std::vector<int>, std::pair<double, double>>>& valid_distributions, LinkAnt* ant, DDCGraph* ddcGraph, Request* req);
	std::vector<std::pair<std::vector<int>, std::pair<double, double>>> enumerate_distributions(int n, int k, LinkAnt* ant, DDCGraph* ddcGraph, Request* req);

private:
	bool searchFin;

	std::pair<std::vector<int>, double> optimize_distribution(int n, int k, std::vector<double>& capacities, Request* req, LinkAnt* ant, DDCGraph* ddcGraph);


};
