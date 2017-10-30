#include "VirtualMachine.h"
#include "Machine.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <stdint.h>
#include <list>
#include <vector>
#include <iostream>
using namespace std;

#ifdef  __cplusplus
extern "C" {
#endif

#define VM_THREAD_ID_IDLE  ((TVMThreadID)0)
#define VM_THREAD_ID_MAIN  ((TVMThreadID)1)

#define VM_THREAD_PRIORITY_IDLE ((TVMThreadPriority)0x00)
#define VM_THREAD_MINIMUM_MEMORY ((TVMMemorySize)0x100000)

#define VM_MEMORY_POOL_ID_SYSTEM ((TVMMemoryPoolID)0)
#define VM_MEMORY_POOL_ID_SHARED ((TVMMemoryPoolID)1)
#define VM_MUTEX_STATE_LOCK ((TVMMutexState)1)
#define VM_MUTEX_STATE_UNLOCK ((TVMMutexState)0)
#define VM_MUTEX_STATE_INVALID ((TVMMutexState)-1)
typedef unsigned int TVMMutexState, *TVMMutexStateRef;

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
    void Initialize(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio);
    void PrepareForActivation();
    void ReleaseAllMutexes();
    static void EntryPoint(void *param);
    static void FileStaticCallback(void *param, int result);
};

class CVMMutex{
  public:
    TVMMutexID DMutexID;
	TVMMutexState DMutexState;
    TVMThreadID DOwnerThread;
	std::vector< std::list< TVMThreadID > > DWaitingThreads;
    
    CVMMutex();
    ~CVMMutex();
	TVMThreadID NextOwner(unsigned int prio);
	unsigned int GetMutexHighestPrio();
    bool RemoveWaiter(TVMThreadID threadid);
};

class CVMPointerLength{
  public:
    TVMMemorySize DLength;
	uint8_t* DBase;
};

class CVMMemoryPool{
  public:
    TVMMemorySize DMemoryPoolSize;
	TVMMemoryPoolID DMemoryPoolID;
	uint8_t* DBasePointer;
	TVMMemorySize DFreeSpace;
    std::vector< CVMPointerLength* > DFreeList;
	std::vector< CVMPointerLength* > DAllocatedList;
    
	CVMMemoryPool();
    CVMMemoryPool(TVMMemorySize MemoryPoolSize, TVMMemoryPoolID MemoryPoolID);
    CVMMemoryPool(TVMMemorySize MemoryPoolSize, TVMMemoryPoolID MemoryPoolID, uint8_t* BasePointer);
	~CVMMemoryPool();
};

CVMMemoryPool::CVMMemoryPool(){
  DMemoryPoolSize = 0;
  DMemoryPoolID = VM_MEMORY_POOL_ID_INVALID;
  DBasePointer = NULL;
  DFreeSpace = 0;
}
CVMMemoryPool::CVMMemoryPool(TVMMemorySize MemoryPoolSize, TVMMemoryPoolID MemoryPoolID){
  DMemoryPoolSize = MemoryPoolSize;
  DMemoryPoolID = MemoryPoolID;
  DBasePointer = new uint8_t[MemoryPoolSize];
  DFreeSpace = MemoryPoolSize;
  CVMPointerLength* FirstFree = new CVMPointerLength;
  FirstFree->DLength = MemoryPoolSize;
  FirstFree->DBase = DBasePointer;
  DFreeList.push_back(FirstFree);

}
CVMMemoryPool::CVMMemoryPool(TVMMemorySize MemoryPoolSize, TVMMemoryPoolID MemoryPoolID, uint8_t* BasePointer){
  DMemoryPoolSize = MemoryPoolSize;
  DMemoryPoolID = MemoryPoolID;
  DBasePointer = BasePointer;
  DFreeSpace = MemoryPoolSize;
  CVMPointerLength* FirstFree = new CVMPointerLength;
  FirstFree->DLength = MemoryPoolSize;
  FirstFree->DBase = BasePointer;
  DFreeList.push_back(FirstFree);

}
CVMMemoryPool::~CVMMemoryPool(){
  if(NULL != DBasePointer){
    delete [] DBasePointer;
  }
}
TVMMutexID VMAllMutexID = 0;

struct transformer{
  int argc;
  char** argv;
};

transformer *transformMainParam=new transformer();
TVMMainEntry MainEntry;

TVMMainEntry VMLoadModule(const char *module);
void VMUnloadModule(void);
void VMReadyQueueAdd(TVMThreadID threadid);
void VMReadyQueueRemove(TVMThreadID threadid);
int VMReadyQueueHighestPriority();

std::vector< CVMThread * > VMAllThreads;
std::vector< CVMMemoryPool * > VMAllMemoryPool;
std::list< CVMThread * > VMWaitingThreads;
std::vector< std::list< CVMThread * > > VMReadyThreads;
std::vector< CVMMutex * > VMAllMutexes;

TVMMemoryPoolID MemoryVectorIndex = 2;
TVMThreadID CurrentRunningThreadID;
volatile TVMThreadID VMCurrentThreadID;

TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer){
  if (size != 0 && pointer != NULL ){
//    cout<< "MemoryBase "<<pointer<<endl;
	std::vector< CVMPointerLength*>::iterator i;
	for (i = VMAllMemoryPool[memory]->DFreeList.begin(); i != VMAllMemoryPool[memory]->DFreeList.end(); i++){
	  if ((*i)->DLength >= ((size+63)/64)*64){
	    *pointer =(void*)(*i)->DBase;
		break;
	  }
	}
    if (i != VMAllMemoryPool[memory]->DFreeList.end()){
	  CVMPointerLength* NewAllocatedPointerLength = new CVMPointerLength;
	  NewAllocatedPointerLength->DBase = (*i)->DBase;
	  NewAllocatedPointerLength->DLength = ((size+63)/64)*64;
	  (*i)->DBase = (*i)->DBase + ((size+63)/64)*64;
	  (*i)->DLength = (*i)->DLength - ((size+63)/64)*64;
	  if((*i)->DLength == 0)
	    VMAllMemoryPool[memory]->DFreeList.erase(i);
	  if (VMAllMemoryPool[memory]->DAllocatedList.empty()){
	    VMAllMemoryPool[memory]->DAllocatedList.push_back(NewAllocatedPointerLength);
	  }
	  else{
	    for (i = VMAllMemoryPool[memory]->DAllocatedList.begin(); i != VMAllMemoryPool[memory]->DAllocatedList.end(); i++){
	     if ((*i)->DBase > NewAllocatedPointerLength->DBase)
		   break;
		}
        VMAllMemoryPool[memory]->DAllocatedList.insert(i, NewAllocatedPointerLength);		 
	  }
//    cout<< "MemoryBase "<<pointer<<endl;
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory, TVMMemorySizeRef bytesleft){
  if (memory >= 0 && memory < VMAllMemoryPool.size() && bytesleft != NULL ){
    std::vector< CVMPointerLength* >::iterator i;
	*bytesleft = 0;
    for (i = VMAllMemoryPool[memory]->DFreeList.begin(); i != VMAllMemoryPool[memory]->DFreeList.end(); i++){
      *bytesleft += (*i)->DLength;
    }
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

void VMMemoryPoolFreeMerge(TVMMemoryPoolID memory, CVMPointerLength* FreePointerLength){
  std::vector< CVMPointerLength* >::iterator i;
  for (i = VMAllMemoryPool[memory]->DFreeList.begin(); i != VMAllMemoryPool[memory]->DFreeList.end(); i++){
    if ((*i)->DBase == FreePointerLength->DBase)
	  break;
  }
  if (i != VMAllMemoryPool[memory]->DFreeList.end()){
    if (i < VMAllMemoryPool[memory]->DFreeList.end()-1){
      if ((*i)->DBase + (*i)->DLength == (*(i))->DBase){
	    (*i)->DLength += (*(i+1))->DLength;
        VMAllMemoryPool[memory]->DFreeList.erase(i+1); 
	  }
	}
    if (i > VMAllMemoryPool[memory]->DFreeList.begin()){
      if ((*(i-1))->DBase + (*(i-1))->DLength == (*i)->DBase){
        (*(i-1))->DLength += (*i)->DLength;
		VMAllMemoryPool[memory]->DFreeList.erase(i);
	  }
	}
  }
}

TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory, void *pointer){
  if (memory >= 0 && memory < VMAllMemoryPool.size() && pointer != NULL ){
//    cout<< " AllocateList size " << VMAllMemoryPool[memory]->DAllocatedList.size() << endl;
	std::vector< CVMPointerLength* >::iterator i;
	for (i = VMAllMemoryPool[memory]->DAllocatedList.begin(); i != VMAllMemoryPool[memory]->DAllocatedList.end(); i++){
//	  cout << " *** " << endl;
	  uint8_t* a = (*i)->DBase;
//	  cout<< (int) i << endl;
//	  cout<< (*i)->DLength<< endl;
//	  cout<< static_cast<const void*> ((*i)->DBase) << endl;
//	  cout<<  pointer <<endl;
	  if ((*i)->DBase == pointer){
        break;
	  }
//	  cout<< " *****  " <<endl;
	}

	if (i != VMAllMemoryPool[memory]->DAllocatedList.end()){
      CVMPointerLength* NewFreePointerLength = new CVMPointerLength;
	  NewFreePointerLength->DBase = (*i)->DBase;
	  NewFreePointerLength->DLength = (*i)->DLength;
	  VMAllMemoryPool[memory]->DAllocatedList.erase(i);
      if (VMAllMemoryPool[memory]->DFreeList.empty()){
        VMAllMemoryPool[memory]->DFreeList.push_back(NewFreePointerLength);
	  }
	  else{
        for (i = VMAllMemoryPool[memory]->DFreeList.begin(); i != VMAllMemoryPool[memory]->DFreeList.end(); i++){
		  if((*i)->DBase > NewFreePointerLength->DBase)
		    break;
		}
		VMAllMemoryPool[memory]->DFreeList.insert(i, NewFreePointerLength);
	  }
      VMMemoryPoolFreeMerge(memory, NewFreePointerLength); 
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  else
	return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory){
  if (memory >= 0 && memory < VMAllMemoryPool.size()){
    if (VMAllMemoryPool[memory]->DAllocatedList.size() == NULL){
	  std::vector< CVMMemoryPool* >::iterator i;
	  for (i = VMAllMemoryPool.begin(); i != VMAllMemoryPool.end(); i++){
	    if ((*i)->DMemoryPoolID == memory)
		  break;
      }
	  VMAllMemoryPool.erase(i);
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INVALID_STATE;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMMemoryPoolCreate(void* base, TVMMemorySize size, TVMMemoryPoolIDRef memory){
  if ( base != NULL && size != 0 && memory != NULL){
    CVMMemoryPool* NewMemoryPool = new CVMMemoryPool(size, MemoryVectorIndex,(uint8_t*)base);
	VMAllMemoryPool.push_back(NewMemoryPool);
	*memory = NewMemoryPool->DMemoryPoolID;
	MemoryVectorIndex++;
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

void IdleLoop(void*){
int i=0;
  while(i >= 0){
    if(!(i%10000)){
      i=0;
    }
    i++;
  }
}
void VMReadyQueueAdd(TVMThreadID threadid){
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_LOW){
    VMReadyThreads[1].push_back(VMAllThreads[threadid]);
  }
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_NORMAL){
    VMReadyThreads[2].push_back(VMAllThreads[threadid]);
  }
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_HIGH){
    VMReadyThreads[3].push_back(VMAllThreads[threadid]);
  }
}

int  VMReadyQueueHighestPriority(){
  if(!VMReadyThreads[3].empty()) return 3;
  else if(!VMReadyThreads[2].empty()) return 2;
  else if(!VMReadyThreads[1].empty()) return 1;
  else return 0;
}
void VMScheduler(void* calldata){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  unsigned int readyInt = VMReadyQueueHighestPriority();
  TVMThreadID  TempID, NextRunningThreadID;
  TempID = CurrentRunningThreadID;
//  cout<< "I'm scheduler"<<endl;
//  cout<< "State " << VMAllThreads[CurrentRunningThreadID]->DState << endl;
  TVMThreadID vi = 0;
  for(vi = 0; vi < VMAllThreads.size(); vi++){
    if (VMAllThreads[vi]->DWaitTicks > 0){
	  VMAllThreads[vi]->DWaitTicks--;
//	  cout << VMAllThreads[vi]->DWaitTicks << endl;
	  if (VMAllThreads[vi]->DWaitTicks == 0){
	    VMAllThreads[vi]->DState = VM_THREAD_STATE_READY;
//	    cout << VMAllThreads[vi]->DState  << endl;
		VMReadyQueueAdd(vi);
	  }
	}
  }
  if(VMAllThreads[CurrentRunningThreadID]->DState == VM_THREAD_STATE_RUNNING){
//   write(1,"Schedule Not Idle\n",19);
//	cout << "Thread "<<CurrentRunningThreadID << endl;
    if (VMAllThreads[CurrentRunningThreadID]->DPriority <= readyInt &&
	(!(readyInt==0 && VMAllThreads[CurrentRunningThreadID]->DPriority==0))){
      NextRunningThreadID = VMReadyThreads[readyInt].front()->DThreadID;
      VMReadyQueueRemove(NextRunningThreadID);
	  VMAllThreads[NextRunningThreadID]->DState = VM_THREAD_STATE_RUNNING;
      VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_READY; 
      if (!(VMAllThreads[CurrentRunningThreadID]->DPriority == VM_THREAD_ID_IDLE))
	    VMReadyQueueAdd(CurrentRunningThreadID);
      CurrentRunningThreadID = NextRunningThreadID;
      VMCurrentThreadID = CurrentRunningThreadID;
      MachineResumeSignals(&sigstate);
	  MachineContextSwitch(&(VMAllThreads[TempID]->DContext),&(VMAllThreads[NextRunningThreadID]->DContext));
    }
    else{
      VMCurrentThreadID = CurrentRunningThreadID;
      MachineResumeSignals(&sigstate);
    }
  }
  else{
//  write(1,"Schedule Idle\n",15);
//	cout << "Thread "<<CurrentRunningThreadID << endl;
    NextRunningThreadID = VMReadyThreads[readyInt].front()->DThreadID;
//	  cout << "CurrentRunning " << CurrentRunningThreadID << " Next " << NextRunningThreadID  << endl;
    if (!(VMAllThreads[NextRunningThreadID]->DPriority == VM_THREAD_ID_IDLE))
	{
//	  cout << "readyInt " << readyInt << "DPriority " << VMAllThreads[NextRunningThreadID] -> DPriority << endl;
      VMReadyQueueRemove(NextRunningThreadID);
	}
	VMAllThreads[NextRunningThreadID]->DState = VM_THREAD_STATE_RUNNING;
    CurrentRunningThreadID = NextRunningThreadID;
    VMCurrentThreadID = CurrentRunningThreadID;
    MachineResumeSignals(&sigstate);
//    write(1,"Idle Resume Sig\n",18);
    
//	  cout << "Temp " << TempID << " Next " << NextRunningThreadID  << endl;
    MachineContextSwitch(&(VMAllThreads[TempID]->DContext),&(VMAllThreads[NextRunningThreadID]->DContext));
  }
 
}

void VMScheduler1(void* calldata){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  unsigned int readyInt = VMReadyQueueHighestPriority();
  TVMThreadID  TempID, NextRunningThreadID;
  TempID = CurrentRunningThreadID;
//  cout<< "I'm scheduler"<<endl;
//  cout<< "State " << VMAllThreads[CurrentRunningThreadID]->DState << endl;
//	  cout << VMAllThreads[vi]->DWaitTicks << endl;
//	    cout << VMAllThreads[vi]->DState  << endl;
  if(VMAllThreads[CurrentRunningThreadID]->DState == VM_THREAD_STATE_RUNNING){
//   write(1,"Schedule Not Idle\n",19);
//	cout << "Thread "<<CurrentRunningThreadID << endl;
    if (VMAllThreads[CurrentRunningThreadID]->DPriority <= readyInt &&
	(!(readyInt==0 && VMAllThreads[CurrentRunningThreadID]->DPriority==0))){
      NextRunningThreadID = VMReadyThreads[readyInt].front()->DThreadID;
      VMReadyQueueRemove(NextRunningThreadID);
	  VMAllThreads[NextRunningThreadID]->DState = VM_THREAD_STATE_RUNNING;
      VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_READY; 
      if (!(VMAllThreads[CurrentRunningThreadID]->DPriority == VM_THREAD_ID_IDLE))
	    VMReadyQueueAdd(CurrentRunningThreadID);
      CurrentRunningThreadID = NextRunningThreadID;
      VMCurrentThreadID = CurrentRunningThreadID;
      MachineResumeSignals(&sigstate);
	  MachineContextSwitch(&(VMAllThreads[TempID]->DContext),&(VMAllThreads[NextRunningThreadID]->DContext));
    }
    else{
      VMCurrentThreadID = CurrentRunningThreadID;
      MachineResumeSignals(&sigstate);
    }
  }
  else{
//  write(1,"Schedule Idle\n",15);
//	cout << "Thread "<<CurrentRunningThreadID << endl;
    NextRunningThreadID = VMReadyThreads[readyInt].front()->DThreadID;
//	  cout << "CurrentRunning " << CurrentRunningThreadID << " Next " << NextRunningThreadID  << endl;
    if (!(VMAllThreads[NextRunningThreadID]->DPriority == VM_THREAD_ID_IDLE))
	{
//	  cout << "readyInt " << readyInt << "DPriority " << VMAllThreads[NextRunningThreadID] -> DPriority << endl;
      VMReadyQueueRemove(NextRunningThreadID);
	}
	VMAllThreads[NextRunningThreadID]->DState = VM_THREAD_STATE_RUNNING;
    CurrentRunningThreadID = NextRunningThreadID;
    VMCurrentThreadID = CurrentRunningThreadID;
    MachineResumeSignals(&sigstate);
//    write(1,"Idle Resume Sig\n",18);
    
//	  cout << "Temp " << TempID << " Next " << NextRunningThreadID  << endl;
    MachineContextSwitch(&(VMAllThreads[TempID]->DContext),&(VMAllThreads[NextRunningThreadID]->DContext));
  }
 
}
void VMReadyQueueRemove(TVMThreadID threadid){
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_LOW){
	VMReadyThreads[1].pop_front();
  }
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_NORMAL) { 
    VMReadyThreads[2].pop_front();
  }
  if( VMAllThreads[threadid]->DPriority == VM_THREAD_PRIORITY_HIGH)
    VMReadyThreads[3].pop_front();
}


CVMThread::CVMThread(){
  DThreadID = VM_THREAD_ID_INVALID;
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
void CVMThread::FileStaticCallback(void *param, int result)
{
  
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  CVMThread* Temp;
  Temp = (CVMThread*)param;
  Temp->DFileReturn = result;
  VMReadyQueueAdd(Temp->DThreadID);
  Temp->DState = VM_THREAD_STATE_READY;
  MachineResumeSignals(&sigstate);
}

TVMStatus VMThreadSleep(TVMTick tick){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
  VMAllThreads[CurrentRunningThreadID]->DWaitTicks = tick;
  MachineResumeSignals(&sigstate);
  VMScheduler1(NULL);
  return 0;
}

void ThreadEntryAndTerminate(void* param){
  ((CVMThread*)param)->DEntry(((CVMThread*)param)->DParameter);
  VMThreadTerminate(((CVMThread*)param)->DThreadID); 
}

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid){
  if (entry != NULL && tid != NULL){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    CVMThread* AppThread = new CVMThread();
    VMAllThreads.push_back(AppThread);
    AppThread->DThreadID = VMAllThreads.size() - 1;
	*tid = AppThread->DThreadID;
    AppThread->DEntry = entry;
    AppThread->DParameter = param;
    AppThread->DMemorySize = memsize;
	uint8_t* allocatedBase;
	VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SYSTEM, memsize, (void**)&allocatedBase);
    AppThread->DMemoryBase =allocatedBase;
    AppThread->DPriority = prio;
    MachineResumeSignals(&sigstate);
	MachineContextCreate(&(AppThread->DContext), ThreadEntryAndTerminate, (void*)AppThread, AppThread->DMemoryBase, AppThread->DMemorySize);
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef state){
  if (thread < (TVMThreadID)VMAllThreads.size()){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    *state = VMAllThreads[thread]->DState;
    MachineResumeSignals(&sigstate);
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_ID;
//  else
//    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMThreadActivate(TVMThreadID thread){
  if (thread < (TVMThreadID)VMAllThreads.size()){
    if (VMAllThreads[thread]->DState == VM_THREAD_STATE_DEAD){
      TMachineSignalState sigstate;
      MachineSuspendSignals(&sigstate);
  	  VMAllThreads[thread]->DState = VM_THREAD_STATE_READY;
      VMReadyQueueAdd(thread);
	  MachineResumeSignals(&sigstate);
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INVALID_STATE;
  }
  else
    return VM_STATUS_ERROR_INVALID_ID;
}

TVMStatus VMThreadDelete(TVMThreadID thread){
  if (thread < (TVMThreadID)VMAllThreads.size()){
    if (VMAllThreads[thread]->DState == VM_THREAD_STATE_DEAD){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
  	std::vector<CVMThread *>::iterator i=VMAllThreads.begin();
	for(unsigned int j=0;j!=thread;j++)
      i++;
	VMAllThreads.erase(i);
    MachineResumeSignals(&sigstate);
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INVALID_STATE;
  }
  else
    return VM_STATUS_ERROR_INVALID_ID;
}

TVMStatus VMThreadTerminate(TVMThreadID thread){
  if (thread < (TVMThreadID)VMAllThreads.size()){
    if (VMAllThreads[thread]->DState != VM_THREAD_STATE_DEAD){
      TMachineSignalState sigstate;
      MachineSuspendSignals(&sigstate);
	  if (VMAllThreads[thread]->DState == VM_THREAD_STATE_READY){
        std::list<CVMThread *>::iterator i;
	    unsigned int IDprio = VMAllThreads[thread]->DPriority;
	    i = VMReadyThreads[IDprio].begin();
	    for(; (*i)->DThreadID != thread; i++);
        VMReadyThreads[IDprio].erase(i);
	  }
      VMAllThreads[thread]->DState = VM_THREAD_STATE_DEAD;
      VMAllThreads[thread]->DWaitingMutex = VM_MUTEX_ID_INVALID;
	  if ((VMAllThreads[thread]->DMutexesHeld).size() != 0){
        std::list< TVMMutexID>::iterator i;
        i = VMAllThreads[thread]->DMutexesHeld.begin();	    
	    for(; i!= VMAllThreads[thread]->DMutexesHeld.end(); i++);
          VMMutexRelease(*i);
        VMAllThreads[thread]->DMutexesHeld.erase(VMAllThreads[thread]->DMutexesHeld.begin(), VMAllThreads[thread]->DMutexesHeld.end());	    
	  }
	  MachineResumeSignals(&sigstate);
	  VMScheduler1(NULL);
	  return VM_STATUS_SUCCESS;
	}
	else
	  return VM_STATUS_ERROR_INVALID_STATE;
  }
  else
    return VM_STATUS_ERROR_INVALID_ID;
}

TVMStatus VMThreadID(TVMThreadIDRef threadref){
  if ( threadref != NULL){
    *threadref = CurrentRunningThreadID;
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMMutexCreate(TVMMutexIDRef mutexref){   
  if (mutexref != NULL){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
    CVMMutex* NewMutex = new CVMMutex;
    VMAllMutexes.push_back(NewMutex);
    NewMutex->DMutexID = VMAllMutexID;
	NewMutex->DMutexState = VM_MUTEX_STATE_UNLOCK;
	*mutexref = NewMutex->DMutexID;
	VMAllMutexID++;
    MachineResumeSignals(&sigstate);
	return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout){
  if (mutex < VMAllMutexes.size()){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
    if (VMAllMutexes[mutex]->DMutexState == VM_MUTEX_STATE_UNLOCK){
	  VMAllMutexes[mutex]->DMutexState = VM_MUTEX_STATE_LOCK;
	  VMAllMutexes[mutex]->DOwnerThread = CurrentRunningThreadID;
      VMAllThreads[CurrentRunningThreadID]->DMutexesHeld.push_back(mutex);
//      cout <<"Mutex ID = "<< VMAllMutexes[mutex]->DMutexID << " state = "<< VMAllMutexes[mutex]->DMutexState << " Ower ID = " << VMAllMutexes[mutex]->DOwnerThread << " Thread ID = "<< VMAllThreads[CurrentRunningThreadID]->DThreadID << " have mutex ID = " << mutex << endl;
	  MachineResumeSignals(&sigstate);
	}
	else{
	  unsigned int DWaitingPrio = VMAllThreads[CurrentRunningThreadID]->DPriority;
	  VMAllMutexes[mutex]->DWaitingThreads[DWaitingPrio].push_back(CurrentRunningThreadID);
      VMAllThreads[CurrentRunningThreadID]->DWaitingMutex = mutex;
      VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
//      cout <<"Mutex ID = "<< VMAllMutexes[mutex]->DMutexID << " state = "<< VMAllMutexes[mutex]->DMutexState << " Ower ID = " << VMAllMutexes[mutex]->DOwnerThread << " Thread ID = "<< VMAllThreads[CurrentRunningThreadID]->DThreadID << " wait for mutex ID = " << mutex << " and "<< " mutex ID "<< VMAllMutexes[mutex]->DMutexID << " has waitingThread ID " << (VMAllMutexes[mutex]->DWaitingThreads[DWaitingPrio].front()) << " Thread ID "<< VMAllThreads[CurrentRunningThreadID]->DThreadID << " is waiting for mutex ID "<< VMAllThreads[CurrentRunningThreadID]->DWaitingMutex << " State is " << VMAllThreads[CurrentRunningThreadID]->DState<< endl;
      MachineResumeSignals(&sigstate);
	  VMScheduler1(NULL);
	}
    return VM_STATUS_SUCCESS;
  }
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

unsigned int CVMMutex::GetMutexHighestPrio(){
  if (!DWaitingThreads[3].empty()) return 3;
  else if (!DWaitingThreads[2].empty()) return 2;
  else if (!DWaitingThreads[1].empty()) return 1;
  else  return 0;
}

TVMThreadID CVMMutex::NextOwner(unsigned int prio){
    TVMThreadID NextID;
    NextID = DWaitingThreads[prio].front();
	DWaitingThreads[prio].pop_front();
	return NextID;
}

TVMStatus VMMutexRelease(TVMMutexID mutex){
  if (mutex < VMAllMutexes.size()){
    if (VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DState == VM_THREAD_STATE_RUNNING){
      TMachineSignalState sigstate;
      MachineSuspendSignals(&sigstate);
//      cout << "Thread ID " << VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DThreadID << " releasing mutex ID " << VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.front();
	  VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.pop_front(); 
	  VMAllMutexes[mutex]->DMutexState = VM_MUTEX_STATE_UNLOCK;
//	  cout << "---mutex ID "<< VMAllMutexes[mutex]->DMutexID << " state is " << VMAllMutexes[mutex]->DMutexState;
	  if (VMAllMutexes[mutex]->GetMutexHighestPrio()){
	    VMAllMutexes[mutex]->DOwnerThread = VMAllMutexes[mutex]->NextOwner(VMAllMutexes[mutex]->GetMutexHighestPrio());
        VMAllMutexes[mutex]->DMutexState = VM_MUTEX_STATE_LOCK;
        VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.push_back(mutex);
        VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DWaitingMutex = VM_MUTEX_ID_INVALID;   
        VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DState = VM_THREAD_STATE_READY;
        VMReadyQueueAdd(VMAllMutexes[mutex]->DOwnerThread);
//        cout << " New owner is thread ID " << VMAllMutexes[mutex]->DOwnerThread << " mutex state is "<< VMAllMutexes[mutex]->DMutexState <<" mutex waiting thread queue size = "<< VMAllMutexes[mutex]->DWaitingThreads[VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DPriority].size()<< " Thread ID "<< VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DThreadID << " has mutex ID " << VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.front() << endl;  
		MachineResumeSignals(&sigstate);
      }
	  else{
        VMAllMutexes[mutex]->DOwnerThread = VM_THREAD_ID_INVALID;
//        cout << " has no owner next" << endl; 
		MachineResumeSignals(&sigstate);
	  }
      return VM_STATUS_SUCCESS;
    }

    else
      return VM_STATUS_ERROR_INVALID_STATE;	
  }
  else
    return VM_STATUS_ERROR_INVALID_ID;
}



CVMMutex::CVMMutex(){
  DMutexID = VM_MUTEX_ID_INVALID;
  DMutexState = VM_MUTEX_STATE_INVALID;
  DOwnerThread = VM_THREAD_ID_INVALID;
  DWaitingThreads.resize(4);
}

CVMMutex::~CVMMutex(){
  if(DWaitingThreads.size() != 0){
    delete [] &DWaitingThreads;
  }
}

bool RemoveWaiter(TVMThreadID threadid){
   return 0;  
}

void changeMain(void*);
void changeIdle(void*);

TVMStatus VMStart(int tickms, TVMMemorySize heapsize, int machinetickms, TVMMemorySize sharedsize, int argc, char *argv[]){
  if(VMLoadModule(argv[0]) == NULL)
    return VM_STATUS_FAILURE;
  else
  { 
    uint8_t* sharebase = (uint8_t*)MachineInitialize(machinetickms, sharedsize);    
    CVMMemoryPool* SharedMemoryPool = new CVMMemoryPool(sharedsize, VM_MEMORY_POOL_ID_SHARED, sharebase); 
    VMReadyThreads.resize(4);
    transformMainParam->argc = argc;
    transformMainParam->argv = argv;
    
	CVMMemoryPool* SystemMemoryPool = new CVMMemoryPool(heapsize, VM_MEMORY_POOL_ID_SYSTEM);
	CVMThread* IdleThread=new CVMThread();
    VMAllMemoryPool.push_back(SystemMemoryPool);
    VMAllMemoryPool.push_back(SharedMemoryPool);
//	cout<< "systempool heapsize ="<< SystemMemoryPool->DMemoryPoolSize << " PoolID ="<< SystemMemoryPool->DMemoryPoolID << " BasePointer =" << static_cast<const void*> (SystemMemoryPool->DBasePointer) << " DFreeList size =" << SystemMemoryPool->DFreeList.size() << endl;
	IdleThread->DThreadID = VM_THREAD_ID_IDLE;    
    IdleThread->Initialize(IdleLoop, (void*)NULL, 1000000, VM_THREAD_PRIORITY_IDLE);
    MainEntry = VMLoadModule(argv[0]);
    CVMThread* MainThread=new CVMThread();
    MainThread->Initialize(changeMain, (void*)transformMainParam, 1000000, VM_THREAD_PRIORITY_NORMAL);
    MainThread->DThreadID = VM_THREAD_ID_MAIN;
    MainThread->DState = VM_THREAD_STATE_RUNNING;
    MachineContextCreate(&(IdleThread->DContext), IdleLoop, (void*)NULL, MainThread->DMemoryBase, MainThread->DMemorySize);
    CurrentRunningThreadID = VM_THREAD_ID_MAIN;
    VMAllThreads.push_back(IdleThread);
    VMAllThreads.push_back(MainThread);
    VMReadyThreads[0].push_back(IdleThread);
    MachineEnableSignals();
	TMachineAlarmCallback CallScheduler;
	CallScheduler = VMScheduler;
	MachineRequestAlarm((useconds_t)(tickms*1000), CallScheduler, NULL);
    MainEntry(transformMainParam->argc, transformMainParam->argv);
    MachineTerminate();
    return VM_STATUS_SUCCESS; 
  }   
}
  
TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
  if (data != NULL && length != NULL){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    TMachineFileCallback callback;
    callback =CVMThread::FileStaticCallback;
    CVMThread* calldata = VMAllThreads[CurrentRunningThreadID];
    VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
	uint8_t* sharedAllocate;
	VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, *length, (void**)&sharedAllocate);
    memcpy(sharedAllocate, data , *length);
	MachineFileWrite(filedescriptor, sharedAllocate, *length, callback, (void*)calldata);
    MachineResumeSignals(&sigstate);
    VMScheduler1(calldata);
	*length = VMAllThreads[CurrentRunningThreadID]->DFileReturn;
	return VM_STATUS_SUCCESS;
  }    
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMFileOpen(const char *filename, int flags, int mode, int* filedescriptor){
  if (filename != NULL && filedescriptor != NULL){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    TMachineFileCallback callback;
    callback =CVMThread::FileStaticCallback;
    CVMThread* calldata = VMAllThreads[CurrentRunningThreadID];
    VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
    MachineFileOpen(filename, flags, mode, callback, calldata);
	MachineResumeSignals(&sigstate);
    VMScheduler1(calldata);
	*filedescriptor = VMAllThreads[CurrentRunningThreadID]->DFileReturn;
	return VM_STATUS_SUCCESS;
  }   
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMFileRead(int filedescriptor, void* data, int* length){
  if (data != NULL && length != NULL){
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    TMachineFileCallback callback;
    callback =CVMThread::FileStaticCallback;
    CVMThread* calldata = VMAllThreads[CurrentRunningThreadID];
    VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
	uint8_t* sharedAllocate;
	VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, *length, (void**)&sharedAllocate);
    MachineFileRead(filedescriptor, sharedAllocate, *length, callback, calldata);
	MachineResumeSignals(&sigstate);
    VMScheduler1(calldata);
    memcpy(data, sharedAllocate, *length);
	*length = VMAllThreads[CurrentRunningThreadID]->DFileReturn;
	return VM_STATUS_SUCCESS;
  }   
  else
    return VM_STATUS_ERROR_INVALID_PARAMETER;
}

TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  TMachineFileCallback callback;
  callback =CVMThread::FileStaticCallback;
  CVMThread* calldata = VMAllThreads[CurrentRunningThreadID];
  VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
  MachineFileSeek(filedescriptor, offset, whence, callback, calldata);
  MachineResumeSignals(&sigstate);
  VMScheduler1(calldata);
  *newoffset = VMAllThreads[CurrentRunningThreadID]->DFileReturn;
  return VM_STATUS_SUCCESS;
}

TVMStatus VMFileClose( int filedescriptor){
  TMachineSignalState sigstate;
  MachineSuspendSignals(&sigstate);
  TMachineFileCallback callback;
  callback =CVMThread::FileStaticCallback;
  CVMThread* calldata = VMAllThreads[CurrentRunningThreadID];
  VMAllThreads[CurrentRunningThreadID]->DState = VM_THREAD_STATE_WAITING;
  MachineFileClose( filedescriptor, callback, calldata);
  MachineResumeSignals(&sigstate);
  VMScheduler1(calldata);
  return VM_STATUS_SUCCESS;
}
void changeMain(void*){
  MainEntry(transformMainParam->argc, transformMainParam->argv);
}
#ifdef __cplusplus
}
#endif
