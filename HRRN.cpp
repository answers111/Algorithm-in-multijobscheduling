#include "stdlib.h"
#include "time.h"
#include "windows.h"
#include <iostream>
#include <string>
using namespace std;

const int PCB_Num = 10;

struct PCB
{
    int pid;
    int priority;
    int neededTime;
    int totalWaitTime;
    int arriveTime;
    float responseRatio;
    struct PCB *next = NULL;
};

PCB*arriveTimeQueue(PCB *pcbHead);
PCB*CreatePCB();
PCB*calWaitTime(PCB*pcbHead);
PCB*HRRN(PCB *pcbHead);
void output(PCB*temp);


int main(){
    PCB*pcbHead = CreatePCB();
    PCB*queue = arriveTimeQueue(pcbHead);
    PCB*temp = queue->next;
    PCB*Queue = HRRN(temp);
    output(Queue);
    system("pause");
    return 0;
} 



PCB*CreatePCB(){
    srand((unsigned)time(NULL));
    PCB *pcbHead = new PCB ;
      
    PCB *p = pcbHead;
    for(int i = 0;i < PCB_Num;i++)
    {       
        PCB *NewPCB = new PCB;
        NewPCB->pid = i+1;        
        NewPCB->neededTime = rand()%49+1;
        NewPCB->arriveTime = rand()%20;
        NewPCB->totalWaitTime = 0;
        NewPCB->next = NULL;
        NewPCB ->responseRatio = 1;
        p->next = NewPCB;
        p = NewPCB;
    }
    return pcbHead;
}


PCB*arriveTimeQueue(PCB *pcbHead){
    PCB *pre,*cur,*pre1,*cur1,*end;

    pre = pcbHead;
    cur = pcbHead->next;
    PCB*dummy = new PCB;
    dummy->next = pcbHead;
    while(cur!=NULL)
    {
        PCB *tmp = dummy;
        if(pre->arriveTime > cur->arriveTime){

            
            while(tmp->next->arriveTime < cur->arriveTime)
                tmp = tmp->next;
            pre->next = cur->next;
            cur->next = tmp->next;
            tmp->next = cur;
            cur = pre->next; 
        }

        
        else{
            pre = pre->next;
            cur = cur->next;
        }
    }

    pre1 = pcbHead->next;
    end = pre1;
    cur1 = pre1->next;
    int arriveNum = 0;
    while(pre1->arriveTime == cur1->arriveTime)
    {
        end = end->next;
        arriveNum++;
        pre1 = pre1->next;
        cur1 = cur1->next;
    }
    pre1 = pcbHead->next;
    cur1 = pre1->next;
     while(arriveNum >= 1 && cur1!=NULL)
    {
        PCB *tmp = dummy;
        if(pre1->neededTime > cur1->neededTime){            
            while(tmp->next->neededTime < cur1->neededTime)
                tmp = tmp->next;
            pre1->next = cur1->next;
            cur1->next = tmp->next;
            tmp->next = cur1;
            cur1 = pre->next; 
        }

        
        else{
            pre1 = pre1->next;
            cur1 = cur1->next;
        }
        arriveNum--;
    }

    return dummy->next;
}

PCB*HRRN(PCB *pcbHead)
{
    int time = 0;

    PCB *operating = pcbHead;
    PCB *count_ptr = pcbHead;
    time = operating->arriveTime;
    
    while(count_ptr->next != NULL)
    {   
        
        time = time + operating->neededTime;

        int locNode = 0; 
        PCB*tmp_la = operating->next ;
        while(time < tmp_la->arriveTime)
            time++;
        while(time >= tmp_la->arriveTime )
           {
                tmp_la->responseRatio = float((time + tmp_la->neededTime - tmp_la->arriveTime)) / float(tmp_la->neededTime);
                tmp_la->totalWaitTime = float((time - tmp_la->arriveTime)) ;
                tmp_la = tmp_la->next;
                locNode++;
                if(tmp_la == NULL)
                break;
           } 

        PCB *pre = operating->next;
        PCB *cur = pre->next;

        while(cur!=tmp_la){

            PCB *tmp = cur;
            if(tmp->responseRatio >= operating->next->responseRatio)
            {               
                pre->next = tmp->next ;
                tmp->next = operating->next;
                operating->next = tmp;
                cur = pre->next;

            }
            else{
                pre = pre->next;
                cur = cur->next;

            }
            
            }
        
        operating = operating ->next;
        count_ptr = count_ptr->next;
        }

        time = time + operating->neededTime;


        return pcbHead;
}

void output(PCB*temp){
    float sum = 0;
    for(int i = 0;i < PCB_Num;i++)
    {
        cout<<"process"<<temp->pid<<" : "<<"arrivalTime = "<<temp->arriveTime<<','<<"neededTime = "<<temp->neededTime<<','<<"totalWaitTime = "<<temp->totalWaitTime<<','<<"responseRatio = "<<temp->responseRatio<<endl;
        cout<<endl;
        sum += temp->totalWaitTime;
        temp = temp->next;
    }

    cout<<"the average waiting time is "<<float(sum/PCB_Num)<<endl;
}