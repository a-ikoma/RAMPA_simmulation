import random

app1_rate=0#High-throughput
app2_rate=0#Low-delay
app3_rate=0#Huge model

case1=True#In Case1, all app rate is the same


for i in range(4):
    save="./case"+str(i+1)+"/"
    if i==0:
        case1=True
    else:
        case1=False
    
    if i==1:
        app1_rate=0.8
        app2_rate=0.1
        app3_rate=0.1
    elif i==2:
        app1_rate=0.1
        app2_rate=0.8
        app3_rate=0.1  
    elif i==3:
        app1_rate=0.1
        app2_rate=0.1
        app3_rate=0.8

    for i in range(2):
        reqNum=0
        f = open(save+'allocSeq'+str(i+1)+".txt", 'w', encoding='UTF-8')
        time=1

        owari=False

        while (True):
            if owari==True:
                break
            reqNum=reqNum+1
            time=time+1

            if case1 == True:
                array = [1, 2, 3]
                randV = random.choice(array)

                if randV==1:
                    array2 = [1, 2]
                    randV2 = random.choice(array2)
                    f.write(str(time)+",allocate,"+str(randV2)+","+str(10)+"\n")
                elif randV==2:
                    array2 = [3, 4]
                    randV2 = random.choice(array2)
                    f.write(str(time)+",allocate,"+str(randV2)+","+str(10)+"\n")
                elif randV==3:
                    array2 = [5, 6]
                    randV2 = random.choice(array2)
                    f.write(str(time)+",allocate,"+str(randV2)+","+str(10)+"\n")

            else:
                rate=random.random()
                if 0<=rate and rate<app1_rate:
                    array = [1, 2]
                    randV = random.choice(array)
                    f.write(str(time)+",allocate,"+str(randV)+","+str(10)+"\n")
                elif app1_rate <= rate and rate < app1_rate + app2_rate:
                    array = [3, 4]
                    randV = random.choice(array)
                    f.write(str(time)+",allocate,"+str(randV)+","+str(10)+"\n")
                else:
                    array = [5, 6]
                    randV = random.choice(array)
                    f.write(str(time)+",allocate,"+str(randV)+","+str(10)+"\n")
            if reqNum==1000:
                owari=True
        f.close()




