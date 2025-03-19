#include "ModelPartition.h"
using namespace std;


ModelPartition::ModelPartition() {
    searchFin = false;
}

bool ModelPartition::ModelPartitioning(LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {

    int n = req->model->opeNum; // �I�y���[�V������
    int k = req->stageNum; // �X�e�[�W��


    std::vector<double> caps;//�e�X�e�[�W�ɑΉ�����GPU�̃������e��

    for (int stage = 0; stage < req->stageNum;stage++) {
        caps.push_back(ddcGraph->graph[ant->resourceMap[stage]].capacity);//�e�X�e�[�W�ɑΉ�����GPU�̃������e��
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

    if (ddcGraph->allocatePolicy != 2) {//EdgePipe����Ȃ�������I�y���[�V�����̔z���͂Ȃ񂾂��Ă悢
        ant->partitionSol = { result.first ,{response,1 / (period * 0.001) } };
    }else {//EdgePipe(policy==2)�̂Ƃ��A�����Ƃ������x���̏��������̂�I��
        ant->partitionSol = { result.first ,{response,1 / (period * 0.001) } };
        ant->allCost = response;//�ړI�֐��������x���������炱����I�[���R�X�g�ŁI
    }

    double a = 1 / (period * 0.001);

    return true;
}




std::pair<std::vector<int>,double> ModelPartition::optimize_distribution(int n, int k, std::vector<double>& capacities, Request* req, LinkAnt* ant, DDCGraph* ddcGraph) {
    vector<vector<double>> dp(n, vector<double>(k, INT_MAX));
    vector<vector<int>> prev(n, vector<int>(k, -1));
    vector<vector<double>> responseTime(n, vector<double>(k, INT_MAX));
    
    //�����ǂ��̎��_��GPU�Ԃ̒ʐM�x���͌���Â����Ă���̂ŁA���z�ŕς��v�f�Ƃ��āA�eGPU���������Ԃ̍ő�l�������C�ɂ��čŏ��x�������Ƃ߂܂��B
    //�����x���Ƃ��͍Ō�ɋ��߂��

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

        
        //�S���P�ڂ̃X�e�[�W�Ɋi�[����ꍇ�̉�������
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
                    //cout << stage_weight << ">" << capacities[j] << "\n";
                    continue;
                }

                double stage_speed = 1000 * (((prefix_flops[i + 1] - prefix_flops[m +1]) / ddcGraph->graph[ant->resourceMap[j]].flops) + (stage_weight / ddcGraph->graph[ant->resourceMap[j]].memBand));

                double maxCurPeriod = max(dp[m][j - 1], stage_speed);


                if ((1 / (maxCurPeriod * 0.001)) < req->model->throughPut) {//�X���[�v�b�g�v���𖞂����Ȃ�
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
            if (ddcGraph->allocatePolicy != 2 && dp[n - 1][k - 1] != INT_MAX) {//EdgePipe(policy==2)�ȊO�͉�������Ό�͉��ł��悢
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




bool ModelPartition::check_capacity(int start, int end, int stage, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {//�������e�ʂ̊m�F
    double sum = 0;
    for (int i = start; i <= end; ++i) {
        //sum += weights[i];
        sum += req->model->memConsume[i];//�X�e�[�W�̃I�y���[�V�����̃�������
    }
    if (stage > 0) {//2�X�e�[�W�ȍ~�ł͑O�̑w�̓��͂��L�^�ł���X�y�[�X���K�v
        sum += req->model->outDataSize[stage - 1]*0.001;//�o�͂�MB���0.001������GB�ɕϊ�
    }

    return sum <= ddcGraph->graph[ant->resourceMap[stage]].capacity;//�X�e�[�W�����s����GPU�̃������e��
}

std::pair<bool,std::pair<double,double>> ModelPartition::check_total_speed(const vector<int>& distribution, LinkAnt* ant, DDCGraph* ddcGraph, Request* req) {//�����x���̊m�F
    //i���X�e�[�W�ԍ��ŁAj���I�y���[�V�����̔ԍ�
    double total_time = 0;
    double maxPeriod = -1;
    int start = 0;
    for (int i = 0; i < distribution.size(); ++i) {
        int end = start + distribution[i] - 1;
        double stage_flop = 0;
        double stage_memory=0;
        for (int j = start; j <= end; ++j) {
            stage_flop += req->model->flopNum[j];//�X�e�[�W����FLOPs�̑��a�����߂�
            stage_memory += req->model->memConsume[j];//�X�e�[�W���̃���������̑��ʂ����߂�
        }
        if (i > 0) {
            stage_memory += req->model->outDataSize[i] * 0.001;
        }

        double stageTime = (stage_flop / ddcGraph->graph[ant->resourceMap[i]].flops + stage_memory/ ddcGraph->graph[ant->resourceMap[i]].memBand) *1000;//FLOPs��FLOPS�Ŋ�����+�������������������ш�Ŋ���GPU���������Ԃ�����
        double communicationTime = 0;//�ʐM����
        if (i < req->stageNum-1) {//�Ō�̃X�e�[�W�̏ꍇ�̓f�[�^�o�͒x�����l���Ȃ�
            //*100000��MB��Byte�ɂȂ������߁B*1000�́A�b��m�b�ɂ��邽��
            communicationTime = ant->resourcePairPropagation[i][i + 1] + (((req->model->outDataSize[i]*100000) / ddcGraph->bandwidth) * 1000);//�����ŒʐM�x�������Z(�]���f�[�^�T�C�Y��req->model->outDataSize[j])
        }
        double stagePeriod = max(stageTime, communicationTime);//�ʐM���Ԃ��������Ԃ̍ő�l���X�e�[�W�̂P�X�e�b�v���ԂƂȂ�

        if (maxPeriod < stagePeriod) {
            maxPeriod = stagePeriod;
        }

        if ((1 / (maxPeriod*0.001)) < req->model->throughPut) {//�X���[�v�b�g�v���𖞂����Ȃ�
            return { false,{-1,-1} };
        }
        start = end + 1;
    }

    for (int i = 0; i < distribution.size(); ++i) {
        double communicationTime = ant->resourcePairPropagation[i][i + 1] + (req->model->outDataSize[i] / ddcGraph->bandwidth);;//�X�e�[�W[i]����̒ʐM�x��
        total_time += maxPeriod + communicationTime;
    }

    if (total_time <= req->acceptableTime) {//�����x���v��������
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