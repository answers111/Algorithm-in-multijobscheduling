#include <iostream>
#include "time.h"
#include "stdlib.h"
#include <queue>
#include <list>
#include "string.h"
#include "windows.h"
#include<math.h>
using namespace std;
struct PCB
{
    int pid;
    int priority;
    int neededTime;
    int totalWaitTime;
    int arrivalTime;
    int usedTime;
    struct PCB *next = NULL;
};

struct Queue{
int priority;    //该队列的优先级
int timeSlice;   //该队列的时间片长度
vector<PCB> PCBqueue;   
};


const int MaxQueueNum = 5;
const int MaxProcessNum = 20;
const int IntervalOfCreatePCB = 50;
const int timeSliceOfTheFirstQueue = 10;

HANDLE mutex,sema;
CRITICAL_SECTION cs;
DWORD ThreadID[MaxProcessNum];

DWORD WINAPI createProcess(LPVOID lp);
int FindTransactionID(DWORD tid);

class MultiLeveledFeedbackQueue
{
    public:
        int CreateQueue();//进程产生函数
        int generator();//进程产生器，通过调用进程产生函数并创建线程来模拟进程的产生
        PCB InitializePCB(int id);//初始化线程
        int scheduler();//进程调度器
        int executer(int Queueid);//进程执行器
        friend DWORD WINAPI createProcess(LPVOID lp);//线程函数 因为要调用队列 所以设为友元函数

    private:
        list<Queue> ListQ ;//分级队列容器
};

int MultiLeveledFeedbackQueue::executer(int Queueid)
{
    list<Queue>::iterator iter = ListQ.begin();
    if(Queueid == 1)
        WaitForSingleObject(mutex,INFINITE);
    for(int i = 1; i < Queueid; i++)
        iter++;
    int availableTime = iter->timeSlice;

    while(availableTime > 0 && !iter->PCBqueue.empty())
    {    
        vector<PCB>::iterator executingPCB = iter->PCBqueue.begin();
        vector<PCB>::iterator nextPCB = executingPCB;
        nextPCB++;
        if(executingPCB->priority < 5)
            executingPCB->priority++;
        if(executingPCB->neededTime - executingPCB->usedTime <=  availableTime )
        {
            int tmp = availableTime;
            availableTime = availableTime - executingPCB->neededTime + executingPCB->usedTime;
            executingPCB->usedTime = executingPCB->neededTime;
            cout<<"process "<<executingPCB->pid<<"( neededTime = "<<executingPCB->neededTime<<" , totalWaitTime = "<<executingPCB->totalWaitTime<<" ,priority = "<<executingPCB->priority<<" ,usedTime = "<<executingPCB->usedTime<<" ) has finished"<<endl;
            cout<<endl;
            cout<<"the time occupied by this process is "<<tmp - availableTime<<", and the timeSlice remains "<<availableTime<<endl;
            nextPCB->totalWaitTime += tmp - availableTime;
            iter->PCBqueue.erase(iter->PCBqueue.begin());
            cout<<endl;
        }
        else
        {
            executingPCB->usedTime += availableTime;
            availableTime = 0;
            cout<<"process "<<executingPCB->pid<<" ( neededTime = "<<executingPCB->neededTime<<" , totalWaitTime = "<<executingPCB->totalWaitTime<<" ,priority = "<<executingPCB->priority<<" usedTime = "<<executingPCB->usedTime<<" ) the rest time = "<<executingPCB->neededTime-executingPCB->usedTime<<endl;
            cout<<endl;
            if( executingPCB->priority < 5)
            cout<<"the process is removed from Queue"<<executingPCB->priority<<" to "<<"Queue"<<executingPCB->priority+1<<endl;
            if(executingPCB->priority == 5)
            cout<<"the process is in Queue"<<executingPCB->priority<<endl;
            cout<<endl;
       
        }
    }

    int WaitTime = iter->timeSlice - availableTime;
    Sleep(WaitTime);
    for(int i = Queueid; i <= MaxQueueNum ; i++)
    {    
        if(iter->PCBqueue.empty())
            break;
        for(int j = 0; j < iter->PCBqueue.size(); j++)
        {
            int times =  iter->PCBqueue.size();
            iter->PCBqueue[j].totalWaitTime += WaitTime;
        }

        iter++;
    }

    if(Queueid == 1)
        ReleaseSemaphore(mutex,1,NULL);

    return 0;
}

int MultiLeveledFeedbackQueue::scheduler()
{
    list<Queue>::iterator iter = ListQ.begin();

    for(int i = 1; i <= MaxQueueNum; i++)
    {
        WaitForSingleObject(sema,INFINITE);
        EnterCriticalSection(&cs);//进入临界区
        while(!iter->PCBqueue.empty())
            {
                int flag = executer(i);
                if(iter->PCBqueue[0].usedTime < iter->PCBqueue[0].neededTime && iter->priority < 5)
                {   list<Queue>::iterator iter1  = iter;
                    iter1++;
                    iter1->PCBqueue.push_back(iter->PCBqueue.front());
                    iter->PCBqueue.erase(iter->PCBqueue.begin());
                }
            }
        iter++;
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int MultiLeveledFeedbackQueue::CreateQueue()
{
        for(int i = 1 ; i <= MaxQueueNum ; i++)
    {
        Queue NewQueue ;
        NewQueue.priority = i;
        NewQueue.timeSlice = timeSliceOfTheFirstQueue * pow(2,i-1);
        cout<<"Queue"<<NewQueue.priority<<" : "<<"priority = "<<NewQueue.priority<<","<<"timeSlice = "<<NewQueue.timeSlice<<endl;
        cout<<endl;
        ListQ.push_back(NewQueue);
    }
    //printf("------------------------------------------------\n");
    return 0; 
}

int MultiLeveledFeedbackQueue::generator(){
    HANDLE thread[MaxProcessNum];
    PCB *pcbHead = new PCB;
    PCB *p = pcbHead;
 
    for(int i = 0; i < MaxProcessNum ; i++)
    {   
        EnterCriticalSection(&cs);
        thread[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)createProcess,this,0,&ThreadID[i]);
        LeaveCriticalSection(&cs);
        ReleaseSemaphore(sema,1,NULL);
        Sleep(50);
    }
	WaitForMultipleObjects(MaxProcessNum,thread,TRUE,INFINITE);  //Wait for all the threads to finish
	for (int i = 0; i < MaxProcessNum; i++)
	     CloseHandle(thread[i]);    
    return 0;
}

PCB MultiLeveledFeedbackQueue::InitializePCB(int id)
{
    srand((id+(unsigned)time(NULL))*25);  
    PCB NewPCB;
    NewPCB.arrivalTime = (id-1) * 50;
    NewPCB.neededTime = rand()%198+2 ;
    NewPCB.totalWaitTime = 0;
    NewPCB.priority = 0;
    NewPCB.usedTime = 0;
    NewPCB.pid = id;
    return NewPCB;
}

DWORD WINAPI createProcess(LPVOID lp){
    MultiLeveledFeedbackQueue *i = (MultiLeveledFeedbackQueue*)lp;
    DWORD tid = GetCurrentThreadId();
    WaitForSingleObject(mutex,INFINITE);

    PCB NewPCB = i->InitializePCB(FindTransactionID(tid));
    list<Queue>::iterator iter;
    iter = i->ListQ.begin();
    iter->PCBqueue.push_back(NewPCB);
    cout<<"process"<<FindTransactionID(tid)<<" ( arrivalTime = "<<NewPCB.arrivalTime<<", neededTime = "<<NewPCB.neededTime<<" )"<<" has been created and entered the Queue1"<<endl;
    cout<<endl;
    cout<<"Queue1 "<<"( priority = "<<iter->priority<<" timeSilce = "<<iter->timeSlice<<" )"<<" has "<<iter->PCBqueue.size()<<" PCB now"<<endl;
    cout<<endl;
    ReleaseSemaphore(mutex,1,NULL);
    return 0;
}

int FindTransactionID(DWORD tid)      
{
	int id = 0;
	for (int i = 0; i < MaxProcessNum ; i++)
		 if (ThreadID[i] == tid) id = i;

	return id+1;
}

int main(){
    mutex = CreateSemaphore(NULL,1,1,NULL);
    sema = CreateSemaphore(NULL, 0,MaxProcessNum, NULL);
    InitializeCriticalSection(&cs);
    MultiLeveledFeedbackQueue Schedul;
    Schedul.CreateQueue();
    Schedul.generator();
    Schedul.scheduler();
    system("pause");
    return 0;
}