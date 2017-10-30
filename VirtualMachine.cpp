#include "VirtualMachine.h"
#include "Machine.h"
#include <cstdio>
#include <unistd.h>
#include <stdint.h>
#include <list>
#include <vector>

#ifdef  __cplusplus
extern "C" {
#endif

#define VM_THREAD_ID_IDLE  ((TVMThreadID)0)
#define VM_THREAD_ID_MAIN  ((TVMThreadID)1)

#define VM_THREAD_PRIORITY_IDLE ((TVMThreadPriority)0x00)
#define VM_THREAD_MINIMUM_MEMORY ((TVMMemorySize)0x100000)

class CVMThread{
  public:
    TVMThreadID DThreadID;
    TVMMemorySize DMemorySize;
    uint8_t *DMemoryBase;  //?
    TVMThreadState DState;
    TVMTick DWaitTicks;  //sleep -1
    TVMMutexID DWaitingMutex;
    std::list< TVMMutexID > DMutexesHeld;
    TVMThreadPriority DPriority;
    SMachineContext DContext;  //
    TVMThreadEntry DEntry;  //
    void *DParameter;  //
    int DFileReturn;  //
    
    CVMThread();
    ~CVMThread();
    
    void Execute();
    void FileCallback( int result);
    void Initialize(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio);
    void PrepareForActivation();
    void ReleaseAllMutexes();
    static void EntryPoint(void *param);
    static void FileStaticCallback(void *param, int result);
};

class CVMMutex{
  public:
    TVMMutexID DMutexID;
    TVMThreadID DOwnerThread;
    std::vector< std::list< TVMThreadID > > DWaitingThreads;
    
    CVMMutex();
    ~CVMMutex();
    
    bool Lock();
    bool Unlock();
    bool RemoveWaiter(TVMThreadID threadid);
};

struct transformer{
  int argc;
  char** argv;
};

transformer *transformMainParam=new transformer();
TVMMainEntry MainEntry;


TVMMainEntry VMLoadModule(const char *module);
void VMUnloadModule(void);
void VMSchedule(void);
void VMReadyQueueAdd(TVMThreadID threadid);
void VMReadyQueueRemove(TVMThreadID threadid);
TVMThreadPriority VMReadyQueueHighestPriority();

std::vector< CVMThread * > VMAllThreads;
std::list< CVMThread * > VMSleepingThreads;
std::vector< std::list< CVMThread * > > VMReadyThreads(3);
std::vector< CVMMutex * > VMAllMutexes;
volatile TVMThreadID VMCurrentThreadID;

void VMReadyQueueAdd(TVMThreadID threadid){
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_LOW)
    VMReadyThreads[0].push_back(VMAllThreads[threadid]);
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_NORMAL)
    VMReadyThreads[1].push_back(VMAllThreads[threadid]);
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_HIGH)
    VMReadyThreads[2].push_back(VMAllThreads[threadid]);
}

void VMReadyQueueRemove(TVMThreadID threadid){
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_LOW)
    VMReadyThreads[0].pop_front();
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_NORMAL)
    VMReadyThreads[1].pop_front();
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_HIGH)
    VMReadyThreads[2].pop_front();
}



CVMThread::CVMThread(){
  DMemorySize = 0;
  DMemoryBase = NULL;
  DState = VM_THREAD_STATE_DEAD;
  DWaitTicks = 0;
  DWaitingMutex = VM_MUTEX_ID_INVALID;
  DPriority = VM_THREAD_PRIORITY_IDLE;
  DEntry = NULL;
  DParameter = NULL;
}

void CVMThread::Initialize(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio){
  
  this->DEntry = entry;
  this->DParameter = param;
  this->DMemorySize = memsize;
  this->DMemoryBase = new uint8_t[memsize];
  this->DPriority = prio;
}
  


CVMThread::~CVMThread(){
  if(NULL != DMemoryBase){
    delete [] DMemoryBase;
  }
}

void CVMThread::Execute(){
  if(NULL != DEntry){
    MachineEnableSignals();
    DEntry(DParameter);
  }
}

void CVMThread::FileCallback( int result){
  DFileReturn = result;
  write(1, "good\n", 5);
  VMReadyQueueAdd(DThreadID);
}
  
void CVMThread::FileStaticCallback(void *param, int result)
{
  CVMThread* Temp;
  Temp = (CVMThread*)param;
  Temp->FileCallback(result);
}

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid){
}

void changeMain(void*);

TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[]){
  if(VMLoadModule(argv[0]) == NULL)
    return VM_STATUS_FAILURE;
  else
  { 
 
    MachineInitialize(machinetickms);    
    transformMainParam->argc = argc-1;
    transformMainParam->argv = argv+1;
    CVMThread* IdleThread=new CVMThread();
    IdleThread->DThreadID = VM_THREAD_ID_IDLE;    
  
  
    MainEntry = VMLoadModule(argv[0]);
    CVMThread* MainThread=new CVMThread();
    MainThread->Initialize(changeMain, (void*)transformMainParam, 1000000, VM_THREAD_PRIORITY_LOW);
    MainThread->DThreadID = VM_THREAD_ID_MAIN;
    MainThread->DState = VM_THREAD_STATE_RUNNING;
    
    SMachineContextRef MainContext = new SMachineContext, IdleContext= new SMachineContext;
    MachineContextCreate(MainContext, changeMain, (void*)transformMainParam, MainThread->DMemoryBase, MainThread->DMemorySize);
    MachineContextSave(MainContext);
    MainThread->DContext = *MainContext;
//    MachineContextCreate(IdleContext, changeMain, (void*)transformMainParam, MainThread->DMemoryBase, MainThread->DMemorySize);
    MachineContextSave(IdleContext);
    IdleThread->DContext = *IdleContext;
    
    VMAllThreads.push_back(IdleThread);
    VMAllThreads.push_back(MainThread);
    MachineEnableSignals();
    MainThread->Execute();
    sleep(1);

    
    
    return VM_STATUS_SUCCESS; 
  }   
}
  
void VMSchedule(void){

}
  
TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
  if (data != NULL && length != NULL){
    TMachineFileCallback callback;
    callback = CVMThread::FileStaticCallback;
    MachineFileWrite(filedescriptor, data, *length, callback, NULL);
    //wait swith context , after call back test length==result
//    if (result> 0)
//      return VM_STATUS_SUCCESS;
//    else
//      return VM_STATUS_FAILURE;
  }    
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

void changeMain(void*){
  MainEntry(transformMainParam->argc, transformMainParam->argv);
}

//void changeIdle(void*){
 // IdleEntry(transformIdleParam->argc, transformIdleParam->argv);
//}
}

