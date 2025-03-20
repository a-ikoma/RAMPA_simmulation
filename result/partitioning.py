from tarfile import PAX_NAME_FIELDS
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import collections
stagePolicy=[[""]]

cases=["case1"]
envs=["result_graph1","result_graph2","result_graph3","result_graph4"]

our="RAMPA"
comp1="NCAR"
comp2="PE"
marker=["^","o","s"]

seqCnt=0

fig = plt.figure(figsize=(15,5))
plt.subplots_adjust(wspace=0.05,hspace=0.6)


for env in envs:

    if env=="result_graph1":
        plt.subplot(1,4,1)
        plt.title("Base cluster")
        plt.ylim(0, 11)
        plt.yticks(fontsize=16)
        plt.xlabel('Number of model partitions',fontsize=16)
        plt.ylabel('Number of allocated services',fontsize=20)
    elif env=="result_graph2":
        plt.subplot(1,4,2)
        plt.title("High bandwidth cluster")
        plt.xlabel('Number of model partitions',fontsize=16)
        plt.tick_params(left=False,labelleft=False)
    elif env=="result_graph3":
        plt.subplot(1,4,3)
        plt.title("High performance cluster")
        plt.xlabel('Number of model partitions',fontsize=16)
        plt.tick_params(left=False,labelleft=False)
    elif env=="result_graph4":
        plt.subplot(1,4,4)
        plt.title("Large cluster")
        plt.xlabel('Number of model partitions',fontsize=16)
        plt.tick_params(left=False,labelleft=False)


    for allocCase in cases:
        data1=[]
        data2=[]
        data3=[]
        data4=[]
        data5=[]
        data6=[]
        x1=[]
        x2=[]
        x3=[]
        x4=[]
        x5=[]
        x6=[]
        time=0

        allocateTime=[[],[],[],[],[]]
        seqCnt=0
        label=[]
        allocatedCnt1=[]
        allocatedCnt2=[]
        allocatedCnt3=[]
        allocatedCnt4=[]
        allocatedCnt5=[]
        allocatedCnt6=[]
        for i in range(2):
            for pol in range(1):
                for stagePol in stagePolicy[pol]:
                    seq = open('./'+env+"/"+allocCase+'/result/result_pol'+str(pol)+str(stagePol)+'_seq'+str(i+1)+'.csv', 'r', encoding='shift-jis')

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
                            allocatedCnt1.append(len(tmp[11].split(" "))-1)
                            x1.append(time)
                        elif tmp[1]=="2":
                            allocatedCnt2.append(len(tmp[11].split(" "))-1)
                            x2.append(time)
                        elif tmp[1]=="3":
                            allocatedCnt3.append(len(tmp[11].split(" "))-1)  
                            x3.append(time)
                        elif tmp[1]=="4":
                            allocatedCnt4.append(len(tmp[11].split(" "))-1)
                            x4.append(time)
                        elif tmp[1]=="5":
                            allocatedCnt5.append(len(tmp[11].split(" "))-1)
                            x5.append(time)
                        elif tmp[1]=="6":
                            allocatedCnt6.append(len(tmp[11].split(" "))-1)  
                            x6.append(time)                                
                        time+=1                      
                    seq.close()
        data1.append(np.array(allocatedCnt1))
        data2.append(np.array(allocatedCnt2))
        data3.append(np.array(allocatedCnt3))
        data4.append(np.array(allocatedCnt4))
        data5.append(np.array(allocatedCnt5))
        data6.append(np.array(allocatedCnt6))

        app1 = collections.Counter(data1[0])
        app2 = collections.Counter(data2[0])
        app3 = collections.Counter(data3[0])
        app4 = collections.Counter(data4[0])
        app5 = collections.Counter(data5[0])
        app6 = collections.Counter(data6[0])
        

        

        plt.bar([num -0.375 for num in app1.keys()],app1.values(),width=0.15,label="Service 1",edgecolor='black')
        plt.bar([num -0.225 for num in app2.keys()],app2.values(),width=0.15,label="Service 2",edgecolor='black')
        plt.bar([num -0.075 for num in app3.keys()],app3.values(),width=0.15,label="Service 3",edgecolor='black')
        plt.bar([num +0.075 for num in app4.keys()],app4.values(),width=0.15,label="Service 4",edgecolor='black')
        plt.bar([num +0.225 for num in app5.keys()],app5.values(),width=0.15,label="Service 5",edgecolor='black')
        plt.bar([num +0.375 for num in app6.keys()],app6.values(),width=0.15,label="Service 6",edgecolor='black')


        plt.xlim(0, 10)
        plt.ylim(0, 150)
        plt.tight_layout()
        plt.xticks([0,1,2,3,4,5,6,7,8,9,10], [0,1,2,3,4,5,6,7,8,9,10],fontsize=16)


fig.legend(['Service 1 (High throughput)', 'Service 2 (High throughput)', 'Service 3 (Low delay)', 'Service 4 (Low delay)', 'Service 5 (Huge model)', 'Service 6 (Huge model)'],loc="upper center",fontsize=16, ncol=3,bbox_to_anchor=(0.5, 1.2))
plt.savefig("./partitions.pdf",dpi=130,bbox_inches = 'tight', pad_inches = 0.05)