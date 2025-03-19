#include "ModelPartition.h"
using namespace std;


ModelPartition::ModelPartition() {
    searchFin = false;
}

bool ModelPartition::ModelPartitioning(LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {

    int n = req->model->opeNum;
    int k = req->stageNum;


    std::vector<double> caps;

    for (int stage = 0; stage < req->stageNum;stage++) {
        caps.push_back(ddcGraph->graph[ant->resourceMap[stage]].capacity);
    }

    std::pair<vector<int>,double> result = optimize_distribution(n, k, caps, req,ant,ddcGraph);


    if (result.first.empty()) {
        return false;
    }

    double period=0;

    if (req->stageNum <= 1) {
        period = result.second;
    }
    else {
        for (int stage = 0; stage < req->stageNum - 1; stage++) {
            period = max(ant->resourcePairPropagation[stage][stage + 1] + ((req->model->outDataSize[stage] * 100000*8) / ddcGraph->bandwidth) * 1000, result.second);
        }
    }

    if ((1 / (period * 0.001)) < req->model->throughPut) {

        return false;
    }



    double response=0;
    for (int stage = 0; stage < req->stageNum - 1; stage++) {
        response += period + ant->resourcePairPropagation[stage][stage + 1] + ((req->model->outDataSize[stage] * 100000 * 8) / ddcGraph->bandwidth) * 1000;
    }
    response += period;

    if (response > req->model->acceptableTime) {
        return false;
    }
    
    std::pair<std::vector<int>, std::pair<double, double>> partitionSol;

    if (ddcGraph->allocatePolicy != 2) {
        ant->partitionSol = { result.first ,{response,1 / (period * 0.001) } };
    }else {
        ant->partitionSol = { result.first ,{response,1 / (period * 0.001) } };
        ant->allCost = response;
    }

    double a = 1 / (period * 0.001);

    return true;
}




std::pair<std::vector<int>,double> ModelPartition::optimize_distribution(int n, int k, std::vector<double>& capacities, Request* req, LinkAnt* ant, DDCGraph* ddcGraph) {
    vector<vector<double>> dp(n, vector<double>(k, INT_MAX));
    vector<vector<int>> prev(n, vector<int>(k, -1));
    vector<vector<double>> responseTime(n, vector<double>(k, INT_MAX));

    vector<double> prefix_mem(n + 1, 0);
    vector<double> prefix_flops(n + 1, 0);

    for (int i = 1; i <= n; i++) {
        prefix_mem[i] = prefix_mem[i - 1] + req->model->memConsume[i - 1];
        prefix_flops[i]= prefix_flops[i - 1] + req->model->flopNum[i - 1];
    }

    for (int i = 0; i < n; i++) {
        if (capacities[0] < prefix_mem[i + 1]) {
            dp[i][0] = INT_MAX;
            responseTime[i][0] = INT_MAX;
            continue;
        }

        double procInStage = 1000 * ((prefix_flops[i + 1] / ddcGraph->graph[ant->resourceMap[0]].flops) + (prefix_mem[i + 1] / ddcGraph->graph[ant->resourceMap[0]].memBand)); 
        responseTime[i][0] = procInStage;
        dp[i][0] = procInStage;
    }
    bool searchFin = false;
    for (int j = 1; j < k; j++) {
        for (int i = j; i < n; i++) {
            for (int m = j - 1; m < i; m++) {
                double stage_weight = prefix_mem[i + 1] - prefix_mem[m + 1] + (req->model->outDataSize[m + 1]) * 0.001;
                if (stage_weight > capacities[j]) {
                    continue;
                }

                double stage_speed = 1000 * (((prefix_flops[i + 1] - prefix_flops[m +1]) / ddcGraph->graph[ant->resourceMap[j]].flops) + (stage_weight / ddcGraph->graph[ant->resourceMap[j]].memBand));

                double maxCurPeriod = max(dp[m][j - 1], stage_speed);


                if ((1 / (maxCurPeriod * 0.001)) < req->model->throughPut) {
                    continue;
                }

                double total_speed = responseTime[m][j - 1] + stage_speed;
                
                if (i < req->stageNum - 1) {
                    total_speed += ant->resourcePairPropagation[i][i + 1] + ((req->model->outDataSize[i] * 100000*8) / ddcGraph->bandwidth) * 1000;
                }

                if (total_speed <= req->model->acceptableTime) {
                    if (maxCurPeriod < dp[i][j]) {

                        dp[i][j] = maxCurPeriod;
                        prev[i][j] = m;
                        responseTime[i][j] = total_speed;
                    }
                }
            }
            if (ddcGraph->allocatePolicy != 2 && dp[n - 1][k - 1] != INT_MAX) {
                searchFin = true;
                break;
            }

            if (searchFin == true) {
                break;
            }
        }
        if (searchFin == true) {
            break;
        }
    }

    if (dp[n - 1][k - 1] == INT_MAX) {
        return {};
    }

    vector<int> result;
    int i = n - 1, j = k - 1;
    while (j >= 0) {
        int prev_i = prev[i][j];
        if (prev_i == -1) {
            prev_i = -1;
        }
        result.push_back(i - prev_i);
        i = prev_i;
        j--;
    }
    reverse(result.begin(), result.end());

    return {result, dp[n - 1][k - 1] };
}




bool ModelPartition::check_capacity(int start, int end, int stage, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {
    double sum = 0;
    for (int i = start; i <= end; ++i) {
        //sum += weights[i];
        sum += req->model->memConsume[i];
    }
    if (stage > 0) {
        sum += req->model->outDataSize[stage - 1]*0.001;
    }

    return sum <= ddcGraph->graph[ant->resourceMap[stage]].capacity;
}

std::pair<bool,std::pair<double,double>> ModelPartition::check_total_speed(const vector<int>& distribution, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {
    double total_time = 0;
    double maxPeriod = -1;
    int start = 0;
    for (int i = 0; i < distribution.size(); ++i) {
        int end = start + distribution[i] - 1;
        double stage_flop = 0;
        double stage_memory=0;
        for (int j = start; j <= end; ++j) {
            stage_flop += req->model->flopNum[j];
            stage_memory += req->model->memConsume[j];
        }
        if (i > 0) {
            stage_memory += req->model->outDataSize[i] * 0.001;
        }

        double stageTime = (stage_flop / ddcGraph->graph[ant->resourceMap[i]].flops + stage_memory/ ddcGraph->graph[ant->resourceMap[i]].memBand) *1000;
        double communicationTime = 0;
        if (i < req->stageNum-1) {
            communicationTime = ant->resourcePairPropagation[i][i + 1] + (((req->model->outDataSize[i]*100000) / ddcGraph->bandwidth) * 1000);
        }
        double stagePeriod = max(stageTime, communicationTime);

        if (maxPeriod < stagePeriod) {
            maxPeriod = stagePeriod;
        }

        if ((1 / (maxPeriod*0.001)) < req->model->throughPut) {
            return { false,{-1,-1} };
        }
        start = end + 1;
    }

    for (int i = 0; i < distribution.size(); ++i) {
        double communicationTime = ant->resourcePairPropagation[i][i + 1] + (req->model->outDataSize[i] / ddcGraph->bandwidth);
        total_time += maxPeriod + communicationTime;
    }

    if (total_time <= req->acceptableTime) {
        return { true, {total_time, (1/(maxPeriod * 0.001))} };
    }else {
        return { false, {-1, -1} };
    }
}

void ModelPartition::backtrack(int n, int k, int start, vector<int>& current_distribution, int remaining_stages,
    vector<std::pair<vector<int>,std::pair<double,double>>>& valid_distributions, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {
    if (remaining_stages == 0) {
        if (start == n) {
            std::pair<bool, std::pair<double, double>> checkConstarint = check_total_speed(current_distribution, ant, ddcGraph, req);
            if (checkConstarint.first == true) {
                valid_distributions.push_back({ current_distribution ,checkConstarint.second});
                if (ddcGraph->allocatePolicy != 3) {
                    searchFin = true;
                }
            }
        }
        return;
    }

    int stage_index = k - remaining_stages;
    for (int end = start; end < n; ++end) {
        if (!check_capacity(start, end, stage_index, ant, ddcGraph, req)) {
            break;
        }

        current_distribution.push_back(end - start + 1);

        std::pair<bool, std::pair<double, double>> checkConstarint = check_total_speed(current_distribution, ant, ddcGraph, req);
        if (checkConstarint.first == true) {
            backtrack(n, k, end + 1, current_distribution, remaining_stages - 1, valid_distributions, ant, ddcGraph, req);
        }

        current_distribution.pop_back();

        if (searchFin == true) {
            return;
        }


    }
}

std::vector<std::pair<std::vector<int>, std::pair<double, double>>> ModelPartition::enumerate_distributions(int n, int k, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {
    std::vector<std::pair<std::vector<int>, std::pair<double, double>>> valid_distributions;
    vector<int> current_distribution;
    backtrack(n, k, 0, current_distribution, k, valid_distributions, ant, ddcGraph, req);
    return valid_distributions;
}