

base1="""antSedai,20
antNum,20
feromonGensui,0.1
feromonParam,2
costParam,1
thread,10
feromonRate,100
initialFeromon,1000
allocatePolicy,1
models,6
log,1
tumekomi,1
"""


base2="""antSedai,20
antNum,20
feromonGensui,0.1
feromonParam,2
costParam,1
thread,10
feromonRate,100
initialFeromon,1000
allocatePolicy,2
models,6
log,1
tumekomi,1
"""



stageApp1=[3,2]
stageApp2=[4,3,2]
stageApp3=[2,1]
stageApp4=[4,3,2]
stageApp5=[9,8,7,6]
stageApp6=[5,4]
cnt=0

for app1 in stageApp1:
    for app2 in stageApp2:
        for app3 in stageApp3:
            for app4 in stageApp4:
                for app5 in stageApp5:
                    for app6 in stageApp6:
                        gen="stage,"+str(app1)+","+str(app2)+","+str(app3)+","+str(app4)+","+str(app5)+","+str(app6)
                        f = open('./param1_'+str(app1)+"-"+str(app2)+"-"+str(app3)+"-"+str(app4)+"-"+str(app5)+"-"+str(app6)+'.txt', 'w', encoding='UTF-8')
                        f.write(base1+gen)
                        f.close()
                        f = open('./param2_'+str(app1)+"-"+str(app2)+"-"+str(app3)+"-"+str(app4)+"-"+str(app5)+"-"+str(app6)+'.txt', 'w', encoding='UTF-8')
                        f.write(base2+gen)
                        f.close()
                        print("set \"stage["+str(cnt)+"]="+str(app1)+"-"+str(app2)+"-"+str(app3)+"-"+str(app4)+"-"+str(app5)+"-"+str(app6)+"\"")
                        cnt+=1

