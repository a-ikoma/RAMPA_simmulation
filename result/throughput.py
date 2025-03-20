from tarfile import PAX_NAME_FIELDS
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import statistics
from statistics import mean
import numpy



def create_defined_arrays(data_avg, yerr):
    # yerr1は [[下側エラー], [上側エラー]] という2次元リスト構造
    
    # 添え字0を除いた配列を取得
    data_without_first = data_avg[1:]
    
    # 最大値、最小値の添え字を取得
    max_val = max(data_without_first)
    min_val = min(data_without_first)
    max_idx = data_without_first.index(max_val) + 1
    min_idx = data_without_first.index(min_val) + 1
    
    # 中央値の添え字を取得
    sorted_data = sorted(data_without_first)
    median_val = sorted_data[len(sorted_data) // 2]
    median_idx = data_without_first.index(median_val) + 1
    
    # 元の添え字0の要素と、抽出した添え字の要素を使って新しい配列を作成
    indices = [0, max_idx, median_idx, min_idx]
    
    # リストで新しい配列を作成
    data_def = [data_avg[i] for i in indices]
    
    # yerr1は2次元リストなので、各次元ごとに処理
    yerr_def = [
        [yerr[0][i] for i in indices],  # 下側エラー
        [yerr[1][i] for i in indices]   # 上側エラー
    ]
    
    return data_def, yerr_def


stagePolicy=[
    [""],
    ["_3-4-2-4-9-5","_3-4-2-4-9-4","_3-4-2-4-8-5","_3-4-2-4-8-4","_3-4-2-4-7-5","_3-4-2-4-7-4","_3-4-2-4-6-5","_3-4-2-4-6-4","_3-4-2-3-9-5","_3-4-2-3-9-4","_3-4-2-3-8-5","_3-4-2-3-8-4","_3-4-2-3-7-5","_3-4-2-3-7-4","_3-4-2-3-6-5","_3-4-2-3-6-4","_3-4-2-2-9-5","_3-4-2-2-9-4","_3-4-2-2-8-5","_3-4-2-2-8-4","_3-4-2-2-7-5","_3-4-2-2-7-4","_3-4-2-2-6-5","_3-4-2-2-6-4","_3-4-1-4-9-5","_3-4-1-4-9-4","_3-4-1-4-8-5","_3-4-1-4-8-4","_3-4-1-4-7-5","_3-4-1-4-7-4","_3-4-1-4-6-5","_3-4-1-4-6-4","_3-4-1-3-9-5","_3-4-1-3-9-4","_3-4-1-3-8-5","_3-4-1-3-8-4","_3-4-1-3-7-5","_3-4-1-3-7-4","_3-4-1-3-6-5","_3-4-1-3-6-4","_3-4-1-2-9-5","_3-4-1-2-9-4","_3-4-1-2-8-5","_3-4-1-2-8-4","_3-4-1-2-7-5","_3-4-1-2-7-4","_3-4-1-2-6-5","_3-4-1-2-6-4","_3-3-2-4-9-5","_3-3-2-4-9-4","_3-3-2-4-8-5","_3-3-2-4-8-4","_3-3-2-4-7-5","_3-3-2-4-7-4","_3-3-2-4-6-5","_3-3-2-4-6-4","_3-3-2-3-9-5","_3-3-2-3-9-4","_3-3-2-3-8-5","_3-3-2-3-8-4","_3-3-2-3-7-5","_3-3-2-3-7-4","_3-3-2-3-6-5","_3-3-2-3-6-4","_3-3-2-2-9-5","_3-3-2-2-9-4","_3-3-2-2-8-5","_3-3-2-2-8-4","_3-3-2-2-7-5","_3-3-2-2-7-4","_3-3-2-2-6-5","_3-3-2-2-6-4","_3-3-1-4-9-5","_3-3-1-4-9-4","_3-3-1-4-8-5","_3-3-1-4-8-4","_3-3-1-4-7-5","_3-3-1-4-7-4","_3-3-1-4-6-5","_3-3-1-4-6-4","_3-3-1-3-9-5","_3-3-1-3-9-4","_3-3-1-3-8-5","_3-3-1-3-8-4","_3-3-1-3-7-5","_3-3-1-3-7-4","_3-3-1-3-6-5","_3-3-1-3-6-4","_3-3-1-2-9-5","_3-3-1-2-9-4","_3-3-1-2-8-5","_3-3-1-2-8-4","_3-3-1-2-7-5","_3-3-1-2-7-4","_3-3-1-2-6-5","_3-3-1-2-6-4","_3-2-2-4-9-5","_3-2-2-4-9-4","_3-2-2-4-8-5","_3-2-2-4-8-4","_3-2-2-4-7-5","_3-2-2-4-7-4","_3-2-2-4-6-5","_3-2-2-4-6-4","_3-2-2-3-9-5","_3-2-2-3-9-4","_3-2-2-3-8-5","_3-2-2-3-8-4","_3-2-2-3-7-5","_3-2-2-3-7-4","_3-2-2-3-6-5","_3-2-2-3-6-4","_3-2-2-2-9-5","_3-2-2-2-9-4","_3-2-2-2-8-5","_3-2-2-2-8-4","_3-2-2-2-7-5","_3-2-2-2-7-4","_3-2-2-2-6-5","_3-2-2-2-6-4","_3-2-1-4-9-5","_3-2-1-4-9-4","_3-2-1-4-8-5","_3-2-1-4-8-4","_3-2-1-4-7-5","_3-2-1-4-7-4","_3-2-1-4-6-5","_3-2-1-4-6-4","_3-2-1-3-9-5","_3-2-1-3-9-4","_3-2-1-3-8-5","_3-2-1-3-8-4","_3-2-1-3-7-5","_3-2-1-3-7-4","_3-2-1-3-6-5","_3-2-1-3-6-4","_3-2-1-2-9-5","_3-2-1-2-9-4","_3-2-1-2-8-5","_3-2-1-2-8-4","_3-2-1-2-7-5","_3-2-1-2-7-4","_3-2-1-2-6-5","_3-2-1-2-6-4","_2-4-2-4-9-5","_2-4-2-4-9-4","_2-4-2-4-8-5","_2-4-2-4-8-4","_2-4-2-4-7-5","_2-4-2-4-7-4","_2-4-2-4-6-5","_2-4-2-4-6-4","_2-4-2-3-9-5","_2-4-2-3-9-4","_2-4-2-3-8-5","_2-4-2-3-8-4","_2-4-2-3-7-5","_2-4-2-3-7-4","_2-4-2-3-6-5","_2-4-2-3-6-4","_2-4-2-2-9-5","_2-4-2-2-9-4","_2-4-2-2-8-5","_2-4-2-2-8-4","_2-4-2-2-7-5","_2-4-2-2-7-4","_2-4-2-2-6-5","_2-4-2-2-6-4","_2-4-1-4-9-5","_2-4-1-4-9-4","_2-4-1-4-8-5","_2-4-1-4-8-4","_2-4-1-4-7-5","_2-4-1-4-7-4","_2-4-1-4-6-5","_2-4-1-4-6-4","_2-4-1-3-9-5","_2-4-1-3-9-4","_2-4-1-3-8-5","_2-4-1-3-8-4","_2-4-1-3-7-5","_2-4-1-3-7-4","_2-4-1-3-6-5","_2-4-1-3-6-4","_2-4-1-2-9-5","_2-4-1-2-9-4","_2-4-1-2-8-5","_2-4-1-2-8-4","_2-4-1-2-7-5","_2-4-1-2-7-4","_2-4-1-2-6-5","_2-4-1-2-6-4","_2-3-2-4-9-5","_2-3-2-4-9-4","_2-3-2-4-8-5","_2-3-2-4-8-4","_2-3-2-4-7-5","_2-3-2-4-7-4","_2-3-2-4-6-5","_2-3-2-4-6-4","_2-3-2-3-9-5","_2-3-2-3-9-4","_2-3-2-3-8-5","_2-3-2-3-8-4","_2-3-2-3-7-5","_2-3-2-3-7-4","_2-3-2-3-6-5","_2-3-2-3-6-4","_2-3-2-2-9-5","_2-3-2-2-9-4","_2-3-2-2-8-5","_2-3-2-2-8-4","_2-3-2-2-7-5","_2-3-2-2-7-4","_2-3-2-2-6-5","_2-3-2-2-6-4","_2-3-1-4-9-5","_2-3-1-4-9-4","_2-3-1-4-8-5","_2-3-1-4-8-4","_2-3-1-4-7-5","_2-3-1-4-7-4","_2-3-1-4-6-5","_2-3-1-4-6-4","_2-3-1-3-9-5","_2-3-1-3-9-4","_2-3-1-3-8-5","_2-3-1-3-8-4","_2-3-1-3-7-5","_2-3-1-3-7-4","_2-3-1-3-6-5","_2-3-1-3-6-4","_2-3-1-2-9-5","_2-3-1-2-9-4","_2-3-1-2-8-5","_2-3-1-2-8-4","_2-3-1-2-7-5","_2-3-1-2-7-4","_2-3-1-2-6-5","_2-3-1-2-6-4","_2-2-2-4-9-5","_2-2-2-4-9-4","_2-2-2-4-8-5","_2-2-2-4-8-4","_2-2-2-4-7-5","_2-2-2-4-7-4","_2-2-2-4-6-5","_2-2-2-4-6-4","_2-2-2-3-9-5","_2-2-2-3-9-4","_2-2-2-3-8-5","_2-2-2-3-8-4","_2-2-2-3-7-5","_2-2-2-3-7-4","_2-2-2-3-6-5","_2-2-2-3-6-4","_2-2-2-2-9-5","_2-2-2-2-9-4","_2-2-2-2-8-5","_2-2-2-2-8-4","_2-2-2-2-7-5","_2-2-2-2-7-4","_2-2-2-2-6-5","_2-2-2-2-6-4","_2-2-1-4-9-5","_2-2-1-4-9-4","_2-2-1-4-8-5","_2-2-1-4-8-4","_2-2-1-4-7-5","_2-2-1-4-7-4","_2-2-1-4-6-5","_2-2-1-4-6-4","_2-2-1-3-9-5","_2-2-1-3-9-4","_2-2-1-3-8-5","_2-2-1-3-8-4","_2-2-1-3-7-5","_2-2-1-3-7-4","_2-2-1-3-6-5","_2-2-1-3-6-4","_2-2-1-2-9-5","_2-2-1-2-9-4","_2-2-1-2-8-5","_2-2-1-2-8-4","_2-2-1-2-7-5","_2-2-1-2-7-4","_2-2-1-2-6-5","_2-2-1-2-6-4"],
    ["_3-4-2-4-9-5","_3-4-2-4-9-4","_3-4-2-4-8-5","_3-4-2-4-8-4","_3-4-2-4-7-5","_3-4-2-4-7-4","_3-4-2-4-6-5","_3-4-2-4-6-4","_3-4-2-3-9-5","_3-4-2-3-9-4","_3-4-2-3-8-5","_3-4-2-3-8-4","_3-4-2-3-7-5","_3-4-2-3-7-4","_3-4-2-3-6-5","_3-4-2-3-6-4","_3-4-2-2-9-5","_3-4-2-2-9-4","_3-4-2-2-8-5","_3-4-2-2-8-4","_3-4-2-2-7-5","_3-4-2-2-7-4","_3-4-2-2-6-5","_3-4-2-2-6-4","_3-4-1-4-9-5","_3-4-1-4-9-4","_3-4-1-4-8-5","_3-4-1-4-8-4","_3-4-1-4-7-5","_3-4-1-4-7-4","_3-4-1-4-6-5","_3-4-1-4-6-4","_3-4-1-3-9-5","_3-4-1-3-9-4","_3-4-1-3-8-5","_3-4-1-3-8-4","_3-4-1-3-7-5","_3-4-1-3-7-4","_3-4-1-3-6-5","_3-4-1-3-6-4","_3-4-1-2-9-5","_3-4-1-2-9-4","_3-4-1-2-8-5","_3-4-1-2-8-4","_3-4-1-2-7-5","_3-4-1-2-7-4","_3-4-1-2-6-5","_3-4-1-2-6-4","_3-3-2-4-9-5","_3-3-2-4-9-4","_3-3-2-4-8-5","_3-3-2-4-8-4","_3-3-2-4-7-5","_3-3-2-4-7-4","_3-3-2-4-6-5","_3-3-2-4-6-4","_3-3-2-3-9-5","_3-3-2-3-9-4","_3-3-2-3-8-5","_3-3-2-3-8-4","_3-3-2-3-7-5","_3-3-2-3-7-4","_3-3-2-3-6-5","_3-3-2-3-6-4","_3-3-2-2-9-5","_3-3-2-2-9-4","_3-3-2-2-8-5","_3-3-2-2-8-4","_3-3-2-2-7-5","_3-3-2-2-7-4","_3-3-2-2-6-5","_3-3-2-2-6-4","_3-3-1-4-9-5","_3-3-1-4-9-4","_3-3-1-4-8-5","_3-3-1-4-8-4","_3-3-1-4-7-5","_3-3-1-4-7-4","_3-3-1-4-6-5","_3-3-1-4-6-4","_3-3-1-3-9-5","_3-3-1-3-9-4","_3-3-1-3-8-5","_3-3-1-3-8-4","_3-3-1-3-7-5","_3-3-1-3-7-4","_3-3-1-3-6-5","_3-3-1-3-6-4","_3-3-1-2-9-5","_3-3-1-2-9-4","_3-3-1-2-8-5","_3-3-1-2-8-4","_3-3-1-2-7-5","_3-3-1-2-7-4","_3-3-1-2-6-5","_3-3-1-2-6-4","_3-2-2-4-9-5","_3-2-2-4-9-4","_3-2-2-4-8-5","_3-2-2-4-8-4","_3-2-2-4-7-5","_3-2-2-4-7-4","_3-2-2-4-6-5","_3-2-2-4-6-4","_3-2-2-3-9-5","_3-2-2-3-9-4","_3-2-2-3-8-5","_3-2-2-3-8-4","_3-2-2-3-7-5","_3-2-2-3-7-4","_3-2-2-3-6-5","_3-2-2-3-6-4","_3-2-2-2-9-5","_3-2-2-2-9-4","_3-2-2-2-8-5","_3-2-2-2-8-4","_3-2-2-2-7-5","_3-2-2-2-7-4","_3-2-2-2-6-5","_3-2-2-2-6-4","_3-2-1-4-9-5","_3-2-1-4-9-4","_3-2-1-4-8-5","_3-2-1-4-8-4","_3-2-1-4-7-5","_3-2-1-4-7-4","_3-2-1-4-6-5","_3-2-1-4-6-4","_3-2-1-3-9-5","_3-2-1-3-9-4","_3-2-1-3-8-5","_3-2-1-3-8-4","_3-2-1-3-7-5","_3-2-1-3-7-4","_3-2-1-3-6-5","_3-2-1-3-6-4","_3-2-1-2-9-5","_3-2-1-2-9-4","_3-2-1-2-8-5","_3-2-1-2-8-4","_3-2-1-2-7-5","_3-2-1-2-7-4","_3-2-1-2-6-5","_3-2-1-2-6-4","_2-4-2-4-9-5","_2-4-2-4-9-4","_2-4-2-4-8-5","_2-4-2-4-8-4","_2-4-2-4-7-5","_2-4-2-4-7-4","_2-4-2-4-6-5","_2-4-2-4-6-4","_2-4-2-3-9-5","_2-4-2-3-9-4","_2-4-2-3-8-5","_2-4-2-3-8-4","_2-4-2-3-7-5","_2-4-2-3-7-4","_2-4-2-3-6-5","_2-4-2-3-6-4","_2-4-2-2-9-5","_2-4-2-2-9-4","_2-4-2-2-8-5","_2-4-2-2-8-4","_2-4-2-2-7-5","_2-4-2-2-7-4","_2-4-2-2-6-5","_2-4-2-2-6-4","_2-4-1-4-9-5","_2-4-1-4-9-4","_2-4-1-4-8-5","_2-4-1-4-8-4","_2-4-1-4-7-5","_2-4-1-4-7-4","_2-4-1-4-6-5","_2-4-1-4-6-4","_2-4-1-3-9-5","_2-4-1-3-9-4","_2-4-1-3-8-5","_2-4-1-3-8-4","_2-4-1-3-7-5","_2-4-1-3-7-4","_2-4-1-3-6-5","_2-4-1-3-6-4","_2-4-1-2-9-5","_2-4-1-2-9-4","_2-4-1-2-8-5","_2-4-1-2-8-4","_2-4-1-2-7-5","_2-4-1-2-7-4","_2-4-1-2-6-5","_2-4-1-2-6-4","_2-3-2-4-9-5","_2-3-2-4-9-4","_2-3-2-4-8-5","_2-3-2-4-8-4","_2-3-2-4-7-5","_2-3-2-4-7-4","_2-3-2-4-6-5","_2-3-2-4-6-4","_2-3-2-3-9-5","_2-3-2-3-9-4","_2-3-2-3-8-5","_2-3-2-3-8-4","_2-3-2-3-7-5","_2-3-2-3-7-4","_2-3-2-3-6-5","_2-3-2-3-6-4","_2-3-2-2-9-5","_2-3-2-2-9-4","_2-3-2-2-8-5","_2-3-2-2-8-4","_2-3-2-2-7-5","_2-3-2-2-7-4","_2-3-2-2-6-5","_2-3-2-2-6-4","_2-3-1-4-9-5","_2-3-1-4-9-4","_2-3-1-4-8-5","_2-3-1-4-8-4","_2-3-1-4-7-5","_2-3-1-4-7-4","_2-3-1-4-6-5","_2-3-1-4-6-4","_2-3-1-3-9-5","_2-3-1-3-9-4","_2-3-1-3-8-5","_2-3-1-3-8-4","_2-3-1-3-7-5","_2-3-1-3-7-4","_2-3-1-3-6-5","_2-3-1-3-6-4","_2-3-1-2-9-5","_2-3-1-2-9-4","_2-3-1-2-8-5","_2-3-1-2-8-4","_2-3-1-2-7-5","_2-3-1-2-7-4","_2-3-1-2-6-5","_2-3-1-2-6-4","_2-2-2-4-9-5","_2-2-2-4-9-4","_2-2-2-4-8-5","_2-2-2-4-8-4","_2-2-2-4-7-5","_2-2-2-4-7-4","_2-2-2-4-6-5","_2-2-2-4-6-4","_2-2-2-3-9-5","_2-2-2-3-9-4","_2-2-2-3-8-5","_2-2-2-3-8-4","_2-2-2-3-7-5","_2-2-2-3-7-4","_2-2-2-3-6-5","_2-2-2-3-6-4","_2-2-2-2-9-5","_2-2-2-2-9-4","_2-2-2-2-8-5","_2-2-2-2-8-4","_2-2-2-2-7-5","_2-2-2-2-7-4","_2-2-2-2-6-5","_2-2-2-2-6-4","_2-2-1-4-9-5","_2-2-1-4-9-4","_2-2-1-4-8-5","_2-2-1-4-8-4","_2-2-1-4-7-5","_2-2-1-4-7-4","_2-2-1-4-6-5","_2-2-1-4-6-4","_2-2-1-3-9-5","_2-2-1-3-9-4","_2-2-1-3-8-5","_2-2-1-3-8-4","_2-2-1-3-7-5","_2-2-1-3-7-4","_2-2-1-3-6-5","_2-2-1-3-6-4","_2-2-1-2-9-5","_2-2-1-2-9-4","_2-2-1-2-8-5","_2-2-1-2-8-4","_2-2-1-2-7-5","_2-2-1-2-7-4","_2-2-1-2-6-5","_2-2-1-2-6-4"]
]#ステージわけの場合分け

cases=["case1"]#バランスよく来る環境
envs=["result_graph1"]#いったん割り当て時間はベース環境だけでいいかな

our="RAMPA"
comp1="NCAR"
comp2="PE"
marker=['+','1', '2', '3', '4',"x","|"]

seqCnt=0
for env in envs:
    for allocCase in cases:
        #全要求
        data1_avg=[]
        data2_avg=[]
        data3_avg=[]
        data4_avg=[]
        data5_avg=[]
        data6_avg=[]

        data1_min=[]
        data2_min=[]
        data3_min=[]
        data4_min=[]
        data5_min=[]
        data6_min=[]

        data1_max=[]
        data2_max=[]
        data3_max=[]
        data4_max=[]
        data5_max=[]
        data6_max=[]


        x1=[]
        x2=[]
        x3=[]
        x4=[]
        x5=[]
        x6=[]
        time=0

        seqCnt=0
        label=[]
        
        for i in range(1):
            for pol in [0,2]:

                for stagePol in stagePolicy[pol]:
                    allocatedCnt1=[]
                    allocatedCnt2=[]
                    allocatedCnt3=[]
                    allocatedCnt4=[]
                    allocatedCnt5=[]
                    allocatedCnt6=[]
                    seq = open('./'+env+"/"+allocCase+'/result/result_pol'+str(pol)+str(stagePol)+'_seq'+str(i+1)+'.csv', 'r', encoding='shift-jis')
                    name=""
                    
                    if pol==0:
                        name+=our
                    elif pol==1:
                        name+=comp1
                        fs=stagePol.split("_")[1].split("-")

                        name+="\n("+fs[0]+","+fs[1]+","+fs[2]+")"
                    elif pol==2:
                        name+=comp2
                        fs=stagePol.split("_")[1].split("-")
                        name+="\n("+fs[0]+","+fs[1]+","+fs[2]+")"
                    

                    
                    label.append(name)





                    while True:
                        data = seq.readline()
                        if "タイムスロット" in data:
                            continue
                        if data == '':
                            break
                        tmp=data.split(",")
                        if tmp[2]!="0":
                            break
                        if tmp[1]=="1":
                            allocatedCnt1.append(float(tmp[10]))
                        elif tmp[1]=="2":
                            allocatedCnt2.append(float(tmp[10]))
                        elif tmp[1]=="3":
                            allocatedCnt3.append(float(tmp[10]))  
                        elif tmp[1]=="4":
                            allocatedCnt4.append(float(tmp[10]))
                        elif tmp[1]=="5":
                            allocatedCnt5.append(float(tmp[10]))
                        elif tmp[1]=="6":
                            allocatedCnt6.append(float(tmp[10]))                              
                        time+=1


                    seq.close()
                    data1_avg.append(mean(allocatedCnt1))
                    data2_avg.append(mean(allocatedCnt2))
                    data3_avg.append(mean(allocatedCnt3))
                    data4_avg.append(mean(allocatedCnt4))
                    data5_avg.append(mean(allocatedCnt5))
                    data6_avg.append(mean(allocatedCnt6))

                    data1_min.append(min(allocatedCnt1))
                    data2_min.append(min(allocatedCnt2))
                    data3_min.append(min(allocatedCnt3))
                    data4_min.append(min(allocatedCnt4))
                    data5_min.append(min(allocatedCnt5))
                    data6_min.append(min(allocatedCnt6))


                    data1_max.append(max(allocatedCnt1))
                    data2_max.append(max(allocatedCnt2))
                    data3_max.append(max(allocatedCnt3))
                    data4_max.append(max(allocatedCnt4))
                    data5_max.append(max(allocatedCnt5))
                    data6_max.append(max(allocatedCnt6))


        print(data1_max)
        yerr1 = [
            [mean - min_val for mean, min_val in zip(data1_avg, data1_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data1_avg, data1_max)]   # 上側のエラー
        ]

        yerr2 = [
            [mean - min_val for mean, min_val in zip(data2_avg, data2_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data2_avg, data2_max)]   # 上側のエラー
        ]

        yerr3 = [
            [mean - min_val for mean, min_val in zip(data3_avg, data3_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data3_avg, data3_max)]   # 上側のエラー
        ]

        yerr4 = [
            [mean - min_val for mean, min_val in zip(data4_avg, data4_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data4_avg, data4_max)]   # 上側のエラー
        ]

        yerr5 = [
            [mean - min_val for mean, min_val in zip(data5_avg, data5_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data5_avg, data5_max)]   # 上側のエラー
        ]

        yerr6 = [
            [mean - min_val for mean, min_val in zip(data6_avg, data6_min)],  # 下側のエラー
            [max_val - mean for mean, max_val in zip(data6_avg, data6_max)]   # 上側のエラー
        ]
        

        #plt.boxplot(data1,labels=label, sym="")
        bar_width = 0.25

        data1_def, yerr1_def = create_defined_arrays(data1_avg,yerr1)
        data2_def, yerr2_def = create_defined_arrays(data2_avg,yerr2)
        data3_def, yerr3_def = create_defined_arrays(data3_avg,yerr3)
        data4_def, yerr4_def = create_defined_arrays(data4_avg,yerr4)
        data5_def, yerr5_def = create_defined_arrays(data5_avg,yerr5)
        data6_def, yerr6_def = create_defined_arrays(data6_avg,yerr6)

        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data1_def,yerr=yerr1_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(60, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 140)

        plt.savefig("./throughput_service1.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)



        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data2_def,yerr=yerr2_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(30, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 120)
        #plt.show()
        plt.savefig("./throughput_service2.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)



        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data3_def,yerr=yerr3_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(30, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 120)
        #plt.show()
        plt.savefig("./throughput_service3.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)



        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data4_def,yerr=yerr4_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(5, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 20)

        plt.savefig("./throughput_service4.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)



        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data5_def,yerr=yerr5_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(0.1, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 5)
        #plt.show()
        plt.savefig("./throughput_service5.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)



        fig, ax = plt.subplots()
        plt.figure(figsize=[5,4.2])
        error_bar_set = dict(lw = 1, capthick = 1, capsize = 2)
        c = ['#377eb8' for _ in range(4)]
        c[0] = '#ff7f00'
        plt.bar(np.arange(4),data6_def,yerr=yerr6_def,ecolor='black',error_kw=error_bar_set,color=c,edgecolor='black')
        plt.axhline(0.25, color="red", linestyle="dashed")


        index = np.arange(len(label))
        plt.legend(["Acceptable throughput"],loc="upper center",fontsize=16, ncol=4,bbox_to_anchor=(0.5, 1.2))
        plt.ylabel('Average throughput (tps)',fontsize=14)
        plt.xticks(np.arange(4),["RAMPA","PE\n(Best)","PE\n(Median)","PE\n(Worst)"],fontsize=10,rotation=90)
        plt.tight_layout()
        plt.ylim(0, 8)
        #plt.show()
        plt.savefig("./throughput_service6.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)


