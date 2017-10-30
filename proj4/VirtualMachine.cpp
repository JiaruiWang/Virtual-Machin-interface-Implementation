#include "VirtualMachine.h"
#include "Machine.h"
#include <stdint.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <list>
#include <vector>
#include<iostream>
#include <math.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
using namespace std;
#ifdef __cplusplus
extern "C" {
#endif
    
#define VM_THREAD_ID_IDLE                    ((TVMThreadID)0)
#define VM_THREAD_ID_MAIN                    ((TVMThreadID)1)
    
#define VM_THREAD_PRIORITY_IDLE                 ((TVMThreadPriority)0x00)
#define VM_THREAD_MINIMUM_MEMORY                 ((TVMMemorySize)0x100000)
const TVMMemoryPoolID VM_MEMORY_POOL_ID_SYSTEM=0;

char CurrentPath[VM_FILE_SYSTEM_MAX_PATH]="/";

class FullDirEntry{
 
   public:
    SVMDirectoryEntry * insideDirEntry;
    unsigned int   DIR_FstClusLO ;
    FullDirEntry();
 

} ;
FullDirEntry::FullDirEntry(){
  
 

}

class FileOpen{


  public:
   int num;
   int clusterCounter;
   int clusterNum;
   int clusterord;
   int halfflag;
   
   FileOpen();
};
FileOpen::FileOpen(){
}


class CVMBootSector{
       public:
         uint32_t jmpBoot;   //34
         string OEMName;    //8
         uint16_t BytsPerSec;   //2
         uint8_t SecPerClus;   //1
         uint16_t RsvdSecCnt;  // 2
         uint8_t NumFATs;    //1
         uint16_t RootEntCnt; //2
         uint16_t TotSec16; //2
         uint8_t Media; //1
         uint16_t FATSz16;  //2
         uint16_t SecPerTrk;  //2 
         uint16_t NumHeads;  //2
         uint32_t HiddSec;   //4
         uint32_t TotSec32;   //4
         uint8_t DrvNum;   //1
         uint8_t Reserved1;  //1
         uint8_t BootSig;  //1
         uint32_t VolID;  //4
         string VolLab;  //11
         string FilSysType;  //8
         CVMBootSector();

};
CVMBootSector::CVMBootSector(){
}

class CurrentPathNode{
   public:
  FullDirEntry* DDirEntry;
 unsigned int pointerToDir;
   uint16_t DIR_FstClusLO;
   std::vector <FullDirEntry* > DAllEntry;
   

   CurrentPathNode();
};
CurrentPathNode::CurrentPathNode(){
  pointerToDir=0;
}
    class CVMThread{
    public:
        TVMThreadID DThreadID;
        TVMMemorySize DMemorySize;
        uint8_t *DMemoryBase;
        TVMThreadState DState;
        TVMTick DWaitTicks;
        TVMMutexID DWaitingMutex;
        std::list < TVMMutexID> DMutexesHeld;
        TVMThreadPriority DPriority;
        SMachineContext DContext;
        TVMThreadEntry DEntry;
        TVMMainEntry   MEntry;
        void *DParameter;
        int DFileReturn;
        
        CVMThread();
        ~CVMThread();
        
        void Execute();
        void FileCallback(int result);
        void Initialize(TVMThreadEntry entry,void *param,TVMMemorySize memsize, TVMThreadPriority prio);
        void PrepareForActivation();
        void ReleaseAllMutexes();
        static void EntryPoint(void *param);
        static void FileStaticCallback(void *param, int result);
    };
    struct alloarea{
    
       // int id;
        uint8_t * base;
        TVMMemorySize allosize;
        TVMMemoryPoolID fatherid;
        
    
    };
    
    
class MemoryPool{
    public:
        uint8_t * startbase;
        TVMMemorySize poolsize;
        TVMMemoryPoolID poolid;
        
        
        std::vector <alloarea *> VMPoolSub;
        std::vector <TVMMemorySize> VMPoolMap;
        MemoryPool();
        ~MemoryPool();   
        bool find(alloarea* newallo,TVMMemorySize size); 
    };
    MemoryPool::MemoryPool(){
    startbase=NULL;
    poolsize=0;   
    }
    
    MemoryPool::~MemoryPool(){
        if(NULL!= startbase)
            delete [] startbase;
    }

bool MemoryPool::find(alloarea* newallo,TVMMemorySize size){
  if(VMPoolMap.size()==1){
    newallo->base=startbase;
    VMPoolSub.push_back(newallo);
    VMPoolMap.insert(VMPoolMap.begin(),0);
    VMPoolMap[1]=VMPoolMap[1]-size;
    return true;
  }
  else{  
    unsigned int i;
   for(i=0;i<VMPoolMap.size();i++){
     if(VMPoolMap[i]>=size){
       newallo->base=VMPoolSub[i-1]->base+VMPoolSub[i-1]->allosize;
       VMPoolSub.insert(VMPoolSub.begin()+i,newallo);
       VMPoolMap.insert(VMPoolMap.begin()+i,0);
       VMPoolMap[i+1]=VMPoolMap[i+1]-size;
//       for(int j=0;j<VMPoolSub.size();j++){
  //       cout<<VMPoolSub[j]->allosize<<""<<j<<" This is Sub Ele \n"<<endl;
    //   }
       return true;
     }
   }
  }
  return false;

}
           
    class CVMMutex{
    public:
        TVMMutexID DMutexID;
        TVMThreadID DOwnerThread;
        int MState;
        std::vector< std::list < TVMThreadID > >  DWaitingThreads;
        CVMMutex();
        ~CVMMutex();
        bool Lock();
        bool Unlock();
        bool RemoveWaiter(TVMThreadID threadid);
    };
    struct Mpara{
        int argc;
        char** argv;
        
    };
    TVMMainEntry   MainEntry;
    TVMMainEntry   VMMain;
    TVMMainEntry VMLoadModule(const char *module);
    void VMUnloadModule(void);
    void VMScheduler(void* param);
    void VMReadyQueueAdd(TVMThreadID ThreadID);
    void VMReadyQueueRemove(TVMThreadID ThreadID);
    void VMSleepingThreadsremove();
    void IdlePro(void* param);
    void BootSectorValueGet(CVMBootSector * FatBS, uint8_t * imagedata);
    void readSector(int totalNum,int offset,uint8_t * Pointer);
    void VMStringCopy(char *dest, const char *src);
    bool IsInDir(const char * path, CurrentPathNode* node);
    SVMDateTime getCurrentTime();
    void writeToFile(uint8_t*content,int offset,int length);
    int StringMatchN(const char *left, const char *right, int n);
    TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory,void *pointer);
    int getFirstFreeCluster();
   TVMStatus VMFileRead2(int filedescriptor, void *data, int *length);
    std::vector < CVMThread * > VMAllThreads;
    std::list < CVMThread * > VMSleepingThreads;
    std::vector < std::list < CVMThread * > > VMReadyThreads(4);
    std::vector <MemoryPool *> VMAllMemoryPool;
    std::vector <uint16_t> VMAllNextClusters;
    std::vector <CurrentPathNode *> CurrentPathRecord;
    std::vector <CurrentPathNode *> VMAllOpenDir;
    CVMBootSector * FatBootSector =new CVMBootSector;
    std::vector <SVMDirectoryEntry *> VMAllDirEntry;
    std::vector <FileOpen*> VMAllFileOpen;
    
    void getFatTable();
    void getDirEntry();
    
    std::vector < CVMMutex * > VMAllMutexes;
    volatile TVMThreadID VMCurrentThreadID;
    int RootDirectorySectors,FirstRootSector,FirstDataSector,ClusterCount;
    int CurrentPathNodeNum;
    CVMMutex::CVMMutex(){
        
        std::list < TVMThreadID> lowT;
        std::list < TVMThreadID> normalT;
        std::list < TVMThreadID> highT;
        DWaitingThreads.push_back(lowT);
        DWaitingThreads.push_back(normalT);
        DWaitingThreads.push_back(highT);
        
    }
    CVMThread::CVMThread(){
        DMemorySize=0;
        DMemoryBase=NULL;
        DState=VM_THREAD_STATE_DEAD;
        DWaitTicks=0;
        DWaitingMutex=VM_MUTEX_ID_INVALID;
        DPriority=VM_THREAD_PRIORITY_IDLE;
        DEntry=NULL;
        DParameter=NULL;
        
    }
    CVMThread::~CVMThread(){
        if(NULL!= DMemoryBase)
            delete [] DMemoryBase;
    }
    
    void CVMThread::Execute(){
        if (NULL!=DEntry) {
            //MachineEnableSignals();
            DEntry(DParameter);
        }
    }
       
    int GetHighestPriority(void){
        if (!VMReadyThreads[2].empty()) return 2;
        else if (!VMReadyThreads[1].empty()) return 1;
        else if (!VMReadyThreads[0].empty()) return 0;
        else return 3;
        
    }

void EntryPoint(void* param){
        CVMThread *pa;
        pa=(CVMThread *)param;
        pa->DEntry(pa->DParameter);
        pa->DState=VM_THREAD_STATE_DEAD;
        VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
        VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
        VMReadyQueueRemove(VMCurrentThreadID);
        MachineContextSwitch(&(pa->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));       
    }
    
    static void FileStaticCallback(void *param, int result){
        CVMThread* temp;
        temp=(CVMThread *) param;
        temp->FileCallback(result);
    }
    void CVMThread::FileCallback(int result) {
        this->DFileReturn=result;
        VMReadyQueueAdd(this->DThreadID);
    }

    
    void VMain(void* param){
        Mpara *pa=new Mpara;
        pa=(Mpara*)param;
        MainEntry(pa->argc,pa->argv);
        
    }
    void CVMThread::PrepareForActivation(){

        
    }
    
   // int poolcounter=1;
TVMStatus VMStart(int tickms, TVMMemorySize heapsize,int machinetickms, TVMMemorySize sharedsize, const char *mount,int argc, char *argv[]){
           
        if(NULL==VMLoadModule(argv[0]))
            return VM_STATUS_FAILURE;
        else
        {
            MemoryPool* sharedMemory=new MemoryPool;
            sharedMemory->startbase=(uint8_t*)MachineInitialize(machinetickms,100*sharedsize);
            sharedMemory->poolsize=100*sharedsize;
            sharedMemory->poolid=1; 
            sharedMemory->VMPoolMap.push_back(100*sharedsize);           
            CVMThread* MainThread=new CVMThread();
            CVMThread* IdleThread=new CVMThread();
            VMAllThreads.push_back(IdleThread);
            VMAllThreads.push_back(MainThread);
            IdleThread->DEntry=IdlePro;
            Mpara *pa=new Mpara;
            pa->argc=argc-1;
            pa->argv=argv+1;
            MainThread->DParameter=(void*)(pa);
            
            MainThread->DEntry=VMain;
            //TotalPoolBase=new uint8_t[heapsize];
            MemoryPool* MainPool=new MemoryPool();
            MainPool->startbase=new uint8_t[heapsize];
            MainPool->poolsize=heapsize;
            MainPool->poolid=VM_MEMORY_POOL_ID_SYSTEM;
            MainPool->VMPoolMap.push_back(heapsize);
            
            VMAllMemoryPool.push_back(MainPool);
            VMAllMemoryPool.push_back(sharedMemory);
            
            MainThread->DThreadID=VM_THREAD_ID_MAIN;
            IdleThread->DThreadID=VM_THREAD_ID_IDLE;
            MainThread->DMemorySize=(TVMMemorySize)0x10000000;
            MainThread->DMemoryBase=new uint8_t[0x10000000];
            IdleThread->DMemorySize=(TVMMemorySize)0x1000000;
            IdleThread->DMemoryBase=new uint8_t[0x1000000];
            MainThread->DPriority=VM_THREAD_PRIORITY_LOW;
            IdleThread->DPriority=VM_THREAD_PRIORITY_IDLE;
            MainThread->MEntry=VMLoadModule(argv[0]);
          //  cout<<VMAllMemoryPool[0]->VMPoolMap[0]<<"\n"<<endl;
            
            SMachineContextRef MainCRef=new SMachineContext;
            SMachineContextRef IdleCRef=new SMachineContext;
            MachineContextCreate(MainCRef,MainThread->DEntry, MainThread->DParameter, MainThread->DMemoryBase, MainThread->DMemorySize);
           // MachineContextSave(&(VMAllThreads[VM_THREAD_ID_MAIN]->DContext));
            MachineContextCreate(IdleCRef,IdleThread->DEntry, IdleThread->DParameter, IdleThread->DMemoryBase, IdleThread->DMemorySize);
           // MachineContextSave(&(VMAllThreads[VM_THREAD_ID_IDLE]->DContext));
            MainThread->DContext=*MainCRef;
            IdleThread->DContext=*IdleCRef;




            //cout<<"in VMstart"<<VMAllThreads.size()<<"\n"<<endl;
            VMReadyThreads[3].push_back(IdleThread);
            //VMReadyQueueAdd(VM_THREAD_ID_MAIN);
            
            VMCurrentThreadID=VM_THREAD_ID_MAIN;
            MainThread->DState=VM_THREAD_STATE_RUNNING;
            MachineEnableSignals();
            TMachineAlarmCallback Scallback;
            Scallback=VMScheduler;
            MachineRequestAlarm((useconds_t)tickms,Scallback, NULL);
            
            
            
            CurrentPathNode * RootNode=new CurrentPathNode();         
            CurrentPathRecord.push_back(RootNode);
            CurrentPathNodeNum=1;
            VMAllOpenDir.push_back(RootNode);
            uint8_t *Pointer;
            VMMemoryPoolAllocate(1, 512, (void **)&Pointer);
            readSector(512,0,Pointer);

            BootSectorValueGet(FatBootSector,Pointer); 
            VMMemoryPoolDeallocate(1,Pointer);
            
        
           getFatTable();
 
           getDirEntry();


            MainThread->MEntry(argc-1,argv+1);
            MachineTerminate();
            return VM_STATUS_SUCCESS;
        }
    }
    
    
    
    TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){
       
        if(NULL!=data&&NULL!=length)
            
        {


            TVMThreadID tempID;
            tempID=VMCurrentThreadID;
            int *Pointer;
            int memorysize=0;
            int j=0;
            if(*length>=512) {memorysize=512;j=512;}
            else {memorysize=*length;j=*length;}
            VMMemoryPoolAllocate(1, memorysize, (void **)&Pointer);
            
            
            while(j!=0){
            *((uint8_t*)Pointer-j+(int)*length)=*((uint8_t*)data+(int)*length-j);
            j--;
            }
            void* calldata;
            //cout<<*length<<endl;
            calldata=(void*)VMAllThreads[tempID];
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            
            VMAllThreads[tempID]->DState=VM_THREAD_STATE_WAITING;
                       
             MachineFileWrite(filedescriptor, Pointer,memorysize,callback, calldata);
            
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMAllThreads[VMCurrentThreadID]->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);

            MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));

           VMMemoryPoolDeallocate(1,(void*)Pointer);
            

            return VM_STATUS_SUCCESS;
          }
        
        else
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        
    }
 /*
 
 
 
 
 
 
    
    TVMStatus VMFileOpen(const char *filename, int flags, int mode,int *filedescriptor){
        
        if(NULL!=filename||NULL!=filedescriptor){
            //cout<<"i am in openning\n"<<endl;
           // MachineContextSave(&(VMAllThreads[VMCurrentThreadID]->DContext));
            void* calldata;
            TVMThreadID tempID;
            tempID=VMCurrentThreadID;
            calldata=(void*)VMAllThreads[tempID];
            
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            MachineFileOpen(filename, flags, mode, callback,calldata);
            // *filedescriptor=VMAllThreads[tempID]->DFileReturn;
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            *filedescriptor=VMAllThreads[tempID]->DFileReturn;
            return VM_STATUS_SUCCESS;
        }
        else
            return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
 */  

    
    
    
    TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset){
        if(newoffset!=NULL){
            MachineContextSave(&(VMAllThreads[VMCurrentThreadID]->DContext));
            void* calldata;
            TVMThreadID tempID;
            tempID=VMCurrentThreadID;
            
            calldata=(void*)VMAllThreads[tempID];
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            MachineFileSeek(filedescriptor, offset,  whence, callback, calldata);
            // *newoffset=VMAllThreads[tempID]->DFileReturn;
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            *newoffset=VMAllThreads[tempID]->DFileReturn;
            return VM_STATUS_SUCCESS;
        }
        else
            return VM_STATUS_FAILURE;
    }
   
    TVMStatus VMFileRead(int filedescriptor, void *data, int *length){
        if(data!=NULL &&length!=NULL){
        
         if(filedescriptor<3){
            MachineContextSave(&(VMAllThreads[VMCurrentThreadID]->DContext));
            void* calldata;
            TVMThreadID tempID;
            tempID=VMCurrentThreadID;
            
            int *Pointer;
            VMMemoryPoolAllocate(1, *length, (void **)&Pointer);

            
            
            calldata=(void*)VMAllThreads[tempID];
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            MachineFileRead(filedescriptor,Pointer, *length, callback, calldata);
            //*length=VMAllThreads[tempID]->DFileReturn;
            
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            *length=VMAllThreads[tempID]->DFileReturn;
            int j=*length;
            while(j!=0){
            *((uint8_t*)data+(int)*length-j)=*((uint8_t*)Pointer-j+(int)*length);
            j--;
            }
            VMMemoryPoolDeallocate(1,(void*)Pointer);
            return VM_STATUS_SUCCESS;
           }
           else{
           
                 
               if(VM_STATUS_SUCCESS==VMFileRead2(filedescriptor, data, length))  return VM_STATUS_SUCCESS;
               return VM_STATUS_FAILURE;
           
           
           }
        }
        else
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        
        
    }
  
    TVMStatus VMFileClose(int filedescriptor){
        if(filedescriptor<3)
        {
        
        MachineContextSave(&(VMAllThreads[VMCurrentThreadID]->DContext));
        void* calldata;
        TVMThreadID tempID;
        tempID=VMCurrentThreadID;
        calldata=(void*)VMAllThreads[tempID];
        TMachineFileCallback callback;
        callback=FileStaticCallback;
        MachineFileClose(filedescriptor, callback, calldata);
        VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
        VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
        VMReadyQueueRemove(VMCurrentThreadID);
        MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
        return VM_STATUS_SUCCESS;
        }
        else{
          VMAllFileOpen[filedescriptor-3]->halfflag=0;
          VMAllFileOpen[filedescriptor-3]->clusterCounter=0;
          VMAllFileOpen[filedescriptor-3]->clusterord=CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[VMAllFileOpen[filedescriptor-3]->num]->DIR_FstClusLO;
        return VM_STATUS_SUCCESS;
        }
        
    }
    
    void VMReadyQueueAdd(TVMThreadID threadid){
        VMAllThreads[threadid]->DState=VM_THREAD_STATE_READY;

        switch(VMAllThreads[threadid]->DPriority)
        {
            case VM_THREAD_PRIORITY_LOW:
                VMReadyThreads[0].push_back(VMAllThreads[threadid]);
                break;
            case VM_THREAD_PRIORITY_NORMAL:
                VMReadyThreads[1].push_back(VMAllThreads[threadid]);
                break;
            case VM_THREAD_PRIORITY_HIGH:
                VMReadyThreads[2].push_back(VMAllThreads[threadid]);
                break;
            default:
                break;
                
        }
       
    }
    void VMReadyQueueRemove(TVMThreadID threadid){
        
        switch(VMAllThreads[threadid]->DPriority)
        {
            case VM_THREAD_PRIORITY_LOW:
                VMReadyThreads[0].pop_front();
                break;
            case VM_THREAD_PRIORITY_NORMAL:
                VMReadyThreads[1].pop_front();
                break;
            case VM_THREAD_PRIORITY_HIGH:
                VMReadyThreads[2].pop_front();
                break;
            case VM_THREAD_PRIORITY_IDLE:
                break;
        }
        
        
    }
    
    void IdlePro(void* param){
        //write(1,"idle pro\n",9);
        //VMCurrentThreadID=VM_THREAD_ID_IDLE;
        //cout<<" in idle"<<"\n"<<endl;
        while(1){
       //cout<<"hello world!"<<endl;
        }
        
    }
    void VMScheduler(void* param){
        
        TMachineSignalStateRef sigstate=NULL;
        MachineSuspendSignals(sigstate);
  // cout<<VMSleepingThreads.size()<<"    \n"<<endl;
       std::list<CVMThread * >::iterator it;
        if(!VMSleepingThreads.empty())
        { //cout<<VMSleepingThreads.size()<<endl;
            
            if(VMSleepingThreads.size()==1){
                VMSleepingThreads.front()->DWaitTicks--;
            
                if(VMSleepingThreads.front()->DWaitTicks==0){
                    
                    
                    VMReadyQueueAdd(VMSleepingThreads.front()->DThreadID);
                    VMSleepingThreads.pop_front();
                    // cout<<VMSleepingThreads.size()<<endl;
                }
            }
            else{
                
                
                for(it=VMSleepingThreads.begin();it!=VMSleepingThreads.end();it++)
                {
                    
                    ((*it)->DWaitTicks)--;
                    
                    if((*it)->DWaitTicks==0)
                    { //cout<<"i am in "<<"\n"<<endl;
                        VMReadyQueueAdd((*it)->DThreadID);
                    }
                }
                
              VMSleepingThreadsremove();
            }
        };
        
     
        if(VMCurrentThreadID==0&&VMReadyThreads[GetHighestPriority()].front()->DThreadID==0)
        { }
        else
        {
            //cout<<VMCurrentThreadID<<"\n"<<endl;
            TVMThreadID temp2ID;
           // MachineContextSave(&(VMAllThreads[VMCurrentThreadID]->DContext));
            VMReadyQueueAdd(VMCurrentThreadID);
            temp2ID=VMCurrentThreadID;
            
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMAllThreads[VMCurrentThreadID]->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
         //   cout<<"switch thread"<<temp2ID<<"  and thread"<< VMCurrentThreadID<< endl;
            MachineContextSwitch(&(VMAllThreads[temp2ID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
        }
        MachineResumeSignals(sigstate);
    }
       
void VMSleepingThreadsremove(){
 unsigned int counter=0;
  std::list<CVMThread * >::iterator it;
   for(it=VMSleepingThreads.begin();it!=VMSleepingThreads.end();it++){
      if((*it)->DWaitTicks==0)
      { VMSleepingThreads.erase(it); break;}
      it++;  
      counter++;
   }
  // cout<<counter<<"  "<< VMSleepingThreads.size()<<"\n"<<endl;
   if(counter<VMSleepingThreads.size()-1)
   {
     VMSleepingThreadsremove();
   }
  
}


    TVMStatus VMThreadSleep(TVMTick tick){
        if (tick==VM_TIMEOUT_INFINITE){return VM_STATUS_ERROR_INVALID_PARAMETER; }
        else
        {
            TVMThreadID temp3ID;
            temp3ID=VMCurrentThreadID;
            VMAllThreads[VMCurrentThreadID]->DWaitTicks=tick;
            VMAllThreads[VMCurrentThreadID]->DState=VM_THREAD_STATE_WAITING;
            
            VMSleepingThreads.push_back(VMAllThreads[VMCurrentThreadID]);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMAllThreads[VMCurrentThreadID]->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[temp3ID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            return VM_STATUS_SUCCESS;
        }
    }
    
    
    TVMStatus VMThreadActivate(TVMThreadID threadid){
    
        if(threadid>=VMAllThreads.size())
            return VM_STATUS_ERROR_INVALID_ID;
        else if(VMAllThreads[threadid]->DState!=VM_THREAD_STATE_DEAD)
            return VM_STATUS_ERROR_INVALID_STATE;
        else
        {
            VMReadyQueueAdd(threadid);
            return VM_STATUS_SUCCESS;
        }
        
    }
    
    TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio,TVMThreadIDRef tid){
        if(entry!=NULL&&tid!=NULL){
            CVMThread* newThread=new CVMThread();
            VMAllThreads.push_back(newThread);
            newThread->DThreadID=VMAllThreads.size()-1;
            
            //cout<<newThread->DThreadID<<"is creating \n"<<endl;
            *tid=newThread->DThreadID;
            //cout<<*tid<<"is creating \n"<<endl;
            //newThread->Initialize(entry,param,memsize,prio);
            
        newThread->DEntry=entry;
        newThread->DParameter=param;
        newThread->DMemorySize=memsize;
        newThread->DMemoryBase=new uint8_t[(int)memsize];
        newThread->DPriority=prio;
        newThread->DState=VM_THREAD_STATE_DEAD;
        MachineContextCreate(&(newThread->DContext), EntryPoint, newThread, newThread->DMemoryBase, newThread->DMemorySize);
        newThread->DWaitTicks=0;
            
            
            return VM_STATUS_SUCCESS;
        }
        else
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        
    }
    
    TVMStatus VMThreadState(TVMThreadID threadid, TVMThreadStateRef state){
        
        if(threadid>=VMAllThreads.size())
            return VM_STATUS_ERROR_INVALID_ID;
        else
        {
            *state=VMAllThreads[threadid]->DState;
            if(state==NULL)
                return VM_STATUS_ERROR_INVALID_PARAMETER;
            else
                return VM_STATUS_SUCCESS;
        }
    }
    TVMStatus VMMutexCreate(TVMMutexIDRef mutexref){
        if(mutexref!=NULL)
        {
            CVMMutex* newMutex=new CVMMutex();
            VMAllMutexes.push_back(newMutex);
            newMutex->DMutexID=VMAllMutexes.size()-1;
            *mutexref=newMutex->DMutexID;
            newMutex->DOwnerThread=-1;
            newMutex->MState=0;
            return VM_STATUS_SUCCESS;
        }
        else 
            return VM_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    void WaitingThreadsAdd(TVMMutexID mutex,TVMThreadID threadid){
        switch(VMAllThreads[threadid]->DPriority)
        {   
            case VM_THREAD_PRIORITY_LOW: 
                VMAllMutexes[mutex]->DWaitingThreads[0].push_back(threadid);
                break;
            case VM_THREAD_PRIORITY_NORMAL:
                VMAllMutexes[mutex]->DWaitingThreads[1].push_back(threadid);
                break;   
            case VM_THREAD_PRIORITY_HIGH:
                VMAllMutexes[mutex]->DWaitingThreads[2].push_back(threadid);
                break; 
            case VM_THREAD_PRIORITY_IDLE:
                break;  
        }   
        
    }
    
    TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout){
        //cout<<"i am in acquire\n"<<endl;
        if(mutex>=VMAllMutexes.size()) return VM_STATUS_ERROR_INVALID_ID;
        
        else{
            if(timeout==VM_TIMEOUT_INFINITE)
            {
                TMachineSignalStateRef sigstate=NULL;
                MachineSuspendSignals(sigstate);
                if (VMAllMutexes[mutex]->MState==1)
                { 
                    TVMThreadID tempID;
                    tempID=VMCurrentThreadID;
                    WaitingThreadsAdd(mutex,VMCurrentThreadID);
                    VMAllThreads[VMCurrentThreadID]->DWaitingMutex=mutex;
                    VMAllThreads[VMCurrentThreadID]->DState=VM_THREAD_STATE_WAITING;
                    VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
                    VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
                    VMReadyQueueRemove(VMCurrentThreadID);
                    MachineContextSwitch(&(VMAllThreads[tempID]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
                    
                }
                else
                {
                    VMAllMutexes[mutex]->DOwnerThread=VMCurrentThreadID;
                    VMAllThreads[VMCurrentThreadID]->DMutexesHeld.push_back(mutex);
                    VMAllMutexes[mutex]->MState=1;
                } 
                MachineResumeSignals(sigstate);
                return VM_STATUS_SUCCESS;
            }
            else return VM_STATUS_SUCCESS;
            
        }
    }
    
    int GetHighestWT(TVMMutexID mutex){
        
        if (!VMAllMutexes[mutex]->DWaitingThreads[2].empty()) return 2;
        else if (!VMAllMutexes[mutex]->DWaitingThreads[1].empty()) 
        return 1;
        else if (!VMAllMutexes[mutex]->DWaitingThreads[0].empty())
         return 0;
        else return -1;
        
    }
    
    TVMStatus VMMutexRelease(TVMMutexID mutex){
               TMachineSignalStateRef sigstate=NULL;
                MachineSuspendSignals(sigstate);
        if(mutex>=VMAllMutexes.size()) return VM_STATUS_ERROR_INVALID_ID;
        
        else 
        { 
            list< TVMMutexID >::iterator itr = VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.begin();
           // cout<<"i am in release1\n"<<endl;
            if(VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.size()==1){
                //cout<<"i am in release2\n"<<endl;
                VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.pop_front();
                //cout<<"i am in release3\n"<<endl;
            }
            else{
            
                while (itr != VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.end()){
                    if(*itr==mutex) {VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.erase(itr);break;}
                    itr++;
                }
                
            }
            
    
            if (GetHighestWT(mutex)!=-1)
            {    
                VMAllMutexes[mutex]->DOwnerThread=VMAllMutexes[mutex]->DWaitingThreads[GetHighestWT(mutex)].front();
                VMAllMutexes[mutex]->DWaitingThreads[GetHighestWT(mutex)].pop_front();
               // cout<<"i am in release4\n"<<endl;
                
               //  cout<<VMAllMutexes[mutex]->DOwnerThread<<"\n"<<endl;
                
                VMAllThreads[VMAllMutexes[mutex]->DOwnerThread]->DMutexesHeld.push_back(mutex);
               // cout<<"i am in release5\n"<<endl;
                //VMAllMutexes[mutex]->DWaitingThreads[GetHighestWT(mutex)].pop_front();
               // cout<<"i am in release6\n"<<endl;
                VMReadyQueueAdd(VMAllMutexes[mutex]->DOwnerThread);
               // cout<<"i am in release7\n"<<endl;
                
                return VM_STATUS_SUCCESS;
            }
            else 
            {
                VMAllMutexes[mutex]->MState=0;
                return VM_STATUS_SUCCESS;
            }
        }
        
         MachineResumeSignals(sigstate);
    }

TVMStatus VMMemoryPoolCreate(void *base, TVMMemorySize size,TVMMemoryPoolIDRef memory){
   if(base==NULL||memory==NULL||size==0) return VM_STATUS_ERROR_INVALID_PARAMETER;
   else 
   {
     MemoryPool* newPool=new MemoryPool();
   unsigned int i=0;
    for(i=0;i<VMAllMemoryPool.size();i++){
      if(VMAllMemoryPool[i]==NULL)
        {
        VMAllMemoryPool[i]=newPool;
        newPool->poolid=(TVMMemoryPoolID)i;
        break;
        }
    }
    
    if(i==VMAllMemoryPool.size()) {VMAllMemoryPool.push_back(newPool); newPool->poolid=(TVMMemoryPoolID)i;}
    
     
     newPool->startbase=(uint8_t*)base;
     newPool->poolsize=size;
     *memory=newPool->poolid;
     newPool->VMPoolMap.push_back(size);
//     cout<<newPool->poolid<<"here are the id\n"<<endl;
     return VM_STATUS_SUCCESS;
   }

}  
TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory){
  if (memory>=VMAllMemoryPool.size()) {return VM_STATUS_ERROR_INVALID_PARAMETER; }
  else{ 
   TVMMemorySize bytesleft;
   VMMemoryPoolQuery( memory,&bytesleft);
   if(bytesleft!=VMAllMemoryPool[memory]->poolsize){return VM_STATUS_ERROR_INVALID_STATE;  }
   else {
    VMAllMemoryPool[memory]=NULL;
    return VM_STATUS_SUCCESS;
   }  
  }
}
 
TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory,TVMMemorySize size, void **pointer){
 if(memory>=VMAllMemoryPool.size()||size==0||pointer==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
 else{
   TVMMemorySize asize;   
   if(round(size/(16*sizeof(int)))==size/(16*sizeof(int)))
    asize=size;
   else
    asize=(round(size/(16*sizeof(int)))+1)*64;   
   TVMMemorySize bytesleft;
   VMMemoryPoolQuery(memory,&bytesleft);
   if(bytesleft<asize){ return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;  }
   else {
   alloarea* newArea = new alloarea;
   newArea->allosize=size;
    if(!VMAllMemoryPool[memory]->find(newArea,asize))    
    {     
     return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
    }
    else{         
     *pointer=newArea->base;
    return VM_STATUS_SUCCESS;
    }     
   } 
 }

}
TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory,void *pointer){
  if(memory>=VMAllMemoryPool.size()||pointer==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
  else {
  unsigned int i=0;
   for( i=0;i<VMAllMemoryPool[memory]->VMPoolSub.size();i++){
     if(VMAllMemoryPool[memory]->VMPoolSub[i]->base==pointer){
  VMAllMemoryPool[memory]->VMPoolMap[i+1]=VMAllMemoryPool[memory]->VMPoolMap[i+1]+VMAllMemoryPool[memory]->VMPoolMap[i]+VMAllMemoryPool[memory]->VMPoolSub[i]->allosize;
       VMAllMemoryPool[memory]->VMPoolMap.erase(VMAllMemoryPool[memory]->VMPoolMap.begin()+i);
       VMAllMemoryPool[memory]->VMPoolSub.erase(VMAllMemoryPool[memory]->VMPoolSub.begin()+i);
       return VM_STATUS_SUCCESS;
     }
   }
   return VM_STATUS_ERROR_INVALID_PARAMETER;  
  }
}   
TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory,TVMMemorySizeRef bytesleft){
  if(memory>=VMAllMemoryPool.size()||bytesleft==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
  else{
    TVMMemorySize tempsize;
    tempsize=(TVMMemorySize)0;
     unsigned int i;
    for(i=0;i<VMAllMemoryPool[memory]->VMPoolMap.size();i++) {
      tempsize=VMAllMemoryPool[memory]->VMPoolMap[i]+tempsize;     
    }
    *bytesleft=tempsize;
    
  }
return VM_STATUS_SUCCESS;
}
void BootSectorValueGet(CVMBootSector * FatBS, uint8_t * imagedata){
   int i;
    FatBS->jmpBoot=(imagedata[0])|(imagedata[1]<<8)|(imagedata[2]<<16);
    //FatBS->OEMName.assign(imagedata+3,8);
    for( i=0;i<8;i++){
       
       FatBS->OEMName.push_back(imagedata[3+i]);
    }
    FatBS->BytsPerSec=(imagedata[11])|(imagedata[12]<<8);
    FatBS->SecPerClus=imagedata[13];
    FatBS->RsvdSecCnt=(imagedata[14])|(imagedata[15]<<8);
    FatBS->NumFATs=imagedata[16];
    FatBS->RootEntCnt=(imagedata[17])|(imagedata[18]<<8);
    FatBS->TotSec16=(imagedata[19])|(imagedata[20]<<8);
    FatBS->Media=imagedata[21];
    FatBS->FATSz16=(imagedata[22])|(imagedata[23]<<8);
    FatBS->SecPerTrk=(imagedata[24])|(imagedata[25]<<8);
    FatBS->NumHeads=(imagedata[26])|(imagedata[27]<<8);
    FatBS->HiddSec=(imagedata[28])|(imagedata[29]<<8)|(imagedata[30]<<16)|(imagedata[31]<<24);
    FatBS->TotSec32=(imagedata[32])|(imagedata[33]<<8)|(imagedata[34]<<16)|(imagedata[35]<<24);
    FatBS->DrvNum=imagedata[36];
    FatBS->Reserved1=imagedata[37];
    FatBS->BootSig=imagedata[38];
    FatBS->VolID=(imagedata[39])|(imagedata[40]<<8)|(imagedata[41]<<16)|(imagedata[42]<<24);
    //FatBS->VolLab.assign(imagedata+43,11);
     for(i=0;i<11;i++){
       
       FatBS->VolLab.push_back(imagedata[43+i]);
    }
    //FatBS->FilSysType.assign(imagedata+54,8);
     for(i=0;i<8;i++){
       
       FatBS->FilSysType.push_back(imagedata[54+i]);
    }
/*    
    
      cout<<"bpb->BS_jmpBoot= "<<FatBS->jmpBoot<<endl;//3
  cout<<"bpb->BS_OEMName= "<<FatBS->OEMName<<endl;//8
  cout<<"BPB_BytsPerSec="<<FatBS->BytsPerSec<<endl;//2
  cout<<"bpb->BPB_SecPerClus="<<(int)FatBS->SecPerClus<<endl;//1
  cout<<"bpb->BPB_RsvdSecCnt="<<FatBS->RsvdSecCnt<<endl;//2
  cout<<"bpb->BPB_NumFATs="<<(int)FatBS->NumFATs<<endl;//1
  cout<<"bpb->BPB_RootEntCnt="<<FatBS->RootEntCnt<<endl;//2
  cout<<"bpb->BPB_TotSec16="<<FatBS->TotSec16<<endl;//2
  cout<<"bpb->BPB_Media="<<(int)FatBS->Media<<endl;//1
  cout<<"bpb->BPB_FATSz16="<<FatBS->FATSz16<<endl;//2
  cout<<"bpb->BPB_SecPerTrk="<<FatBS->SecPerTrk<<endl;//2
  cout<<"bpb->BPB_NumHeads="<<FatBS->NumHeads<<endl;//2
  cout<<"bpb->BPB_HiddSec="<<FatBS->HiddSec<<endl;//4
  cout<<"bpb->BPB_TotSec32="<<FatBS->TotSec32<<endl;//4
  //Fat12 and Fat16 Structure Starting at Offset 36
  cout<<(int)FatBS->DrvNum<<endl;//1
  cout<<(int)FatBS->Reserved1<<endl;//1
  cout<<(int)FatBS->BootSig<<endl;//1
  cout<<FatBS->VolID<<endl;//4
  cout<<FatBS->VolLab<<endl;//11
  cout<<FatBS->FilSysType<<endl;//8
*/  
    
    
    
    FirstRootSector = FatBS->RsvdSecCnt + FatBS->NumFATs * FatBS->FATSz16;
    RootDirectorySectors = (FatBS->RootEntCnt * 32) / 512;
    FirstDataSector = FirstRootSector + RootDirectorySectors;
    ClusterCount = (FatBS->TotSec32 - FirstDataSector) /FatBS->SecPerClus;

}
void getFatTable(){

      int FatBytes=(FirstRootSector-FatBootSector->RsvdSecCnt)*512;
      uint8_t * Pointer;
      uint16_t tempCluster;

      VMMemoryPoolAllocate(1, FatBytes, (void **)&Pointer);
  
      readSector(FatBytes,512,Pointer);
      int i,j;
      

      
     for(i=0;i<2;i++){
         for(j=4;j<(FatBytes)/2;j=j+2){
         
         tempCluster=Pointer[i*FatBytes/2+j]|Pointer[i*FatBytes/2+j+1]<<8;
         VMAllNextClusters.push_back(tempCluster);
         //cout<<hex<<tempCluster<<endl;
         
         }
         
      
      }
      
      
      VMMemoryPoolDeallocate(1,Pointer);

}
void readSector(int totalNum,int offset,uint8_t * Pointer){

             void* calldata;

            calldata=(void*)VMAllThreads[1];
            
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            
            //open
            MachineFileOpen("fat.ima", O_RDWR, 0644, callback, calldata);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMAllThreads[1]->DState=VM_THREAD_STATE_WAITING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[0]->DContext));
 
            int filedescriptor=VMAllThreads[1]->DFileReturn;

            //seek
            MachineFileSeek(filedescriptor, offset,  0, callback, calldata);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            
            //read
            int i;
            for(i=0;i<((totalNum-1)/512+1);i++){
             MachineFileRead(filedescriptor,Pointer+512*i,512,callback,calldata);
             VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMAllThreads[1]->DState=VM_THREAD_STATE_WAITING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[0]->DContext));
            }
            
            //close
            MachineFileClose(filedescriptor, callback, calldata);
           VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
           VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
           VMReadyQueueRemove(VMCurrentThreadID);
           MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));

}

void getDirEntry(){
   
   int RootBytes=(FirstDataSector-FirstRootSector)*512;

   uint8_t *Pointer;
   VMMemoryPoolAllocate(1, RootBytes, (void **)&Pointer);
   readSector(RootBytes,FirstRootSector*512,Pointer);
   int i,j;
   for(i=0;i<FatBootSector->RootEntCnt;i++){
   
   
    if(Pointer[32*i]==0x00) break;
       else if (Pointer[32*i]==0xe5) ;
       else{
      SVMDirectoryEntry* tempDirEntry=new SVMDirectoryEntry;
     FullDirEntry* tempFullDir=new FullDirEntry();
     tempFullDir->insideDirEntry=tempDirEntry;
      
     tempFullDir->DIR_FstClusLO=Pointer[26+32*i]|Pointer[27+32*i]<<8;
      
      for( j=0;j<11;j++){       
       tempDirEntry->DShortFileName[j]=Pointer[j+32*i];
      }
      tempDirEntry->DShortFileName[j]='\0';
     // cout<<"tempDirEntry->DIR_Name    "<<tempDirEntry->DIR_Name<<endl;
      tempDirEntry->DAttributes=Pointer[11+32*i];
       tempDirEntry->DCreate.DDay=Pointer[16+32*i]&0x1f;
      tempDirEntry->DCreate.DMonth=(Pointer[16+32*i]|Pointer[17+32*i]<<8)>>5&0xf;
      tempDirEntry->DCreate.DYear=(Pointer[17+32*i]>>1)+1980;
      //cout<<"   "<<(int)tempDirEntry->DCreate.DDay<<endl;
      //cout<<"   "<<(int)tempDirEntry->DCreate.DMonth<<endl;
      //cout<<"   "<<tempDirEntry->DCreate.DYear<<endl;
      tempDirEntry->DCreate.DSecond=Pointer[14+32*i]&0x1f;
      tempDirEntry->DCreate.DMinute=(Pointer[14+32*i]|Pointer[15+32*i]<<8)>>5&0x3f;
      tempDirEntry->DCreate.DHour=Pointer[15+32*i]>>3;
      
       tempDirEntry->DAccess.DDay=Pointer[18+32*i]&0x1f;
      tempDirEntry->DAccess.DMonth=(Pointer[18+32*i]|Pointer[19+32*i]<<8)>>5&0xf;
      tempDirEntry->DAccess.DYear=(Pointer[19+32*i]>>1)+1980;
      
      
      tempDirEntry->DModify.DSecond=Pointer[22+32*i]&0x1f;
      tempDirEntry->DModify.DMinute=(Pointer[22+32*i]|Pointer[23+32*i]<<8)>>5&0x3f;
      tempDirEntry->DModify.DHour=Pointer[23+32*i]>>3;
      
      
      tempDirEntry->DModify.DDay=Pointer[24+32*i]&0x1f;
      tempDirEntry->DModify.DMonth=(Pointer[24+32*i]|Pointer[25+32*i]<<8)>>5&0xf;
      tempDirEntry->DModify.DYear=(Pointer[25+32*i]>>1)+1980;
      tempDirEntry->DSize=Pointer[28+32*i]|Pointer[29+32*i]<<8|Pointer[30+32*i]<<16|Pointer[31+32*i]<<24;
      //cout<<(int)tempDirEntry->DAttributes<<endl;
      if(tempDirEntry->DAttributes!=0x0f){
      VMAllDirEntry.push_back(tempDirEntry);
      CurrentPathRecord[0]->DAllEntry.push_back(tempFullDir);
      
      }
      }
   
   }
   
   
   VMMemoryPoolDeallocate(1,Pointer);
   
   
}
TVMStatus VMDirectoryCurrent(char *abspath){
  
    if (abspath==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
    else {
      VMStringCopy(abspath,CurrentPath);

      return VM_STATUS_SUCCESS;
    }
    
   

}


TVMStatus VMDirectoryOpen(const char *dirname, int *dirdescriptor){
  if(dirname==NULL||dirdescriptor==NULL)  return VM_STATUS_ERROR_INVALID_PARAMETER;
  else {
  
  
        *dirdescriptor=VMAllOpenDir.size();
        
        VMAllOpenDir.push_back(CurrentPathRecord[CurrentPathRecord.size()-1]);
    if(VMAllOpenDir[VMAllOpenDir.size()-1]->pointerToDir==VMAllOpenDir[VMAllOpenDir.size()-1]->DAllEntry.size())  VMDirectoryRewind(VMAllOpenDir.size()-1);
             
    return VM_STATUS_SUCCESS; 
    }
  
  }
  
TVMStatus VMDirectoryRead(int dirdescriptor,SVMDirectoryEntryRef dirent){
    
    if(dirent==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
    else{
         
   
         
   
         if(VMAllOpenDir[dirdescriptor]->pointerToDir<VMAllOpenDir[dirdescriptor]->DAllEntry.size()){
         *dirent=*(VMAllOpenDir[dirdescriptor]->DAllEntry[VMAllOpenDir[dirdescriptor]->pointerToDir]->insideDirEntry);
             VMAllOpenDir[dirdescriptor]->pointerToDir++;
             return VM_STATUS_SUCCESS;
         }
         else{
     
           return VM_STATUS_FAILURE;
         }
    }
   
  //VMAllOpenDir[dirdescriptor]

}  
  

TVMStatus VMDirectoryChange(const char *path){
  if(path==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
  else {
  
    if(path[0]=='/'&&path[1]=='\0'){
     while(CurrentPathNodeNum!=1){     
      CurrentPathRecord.pop_back();
      CurrentPathNodeNum--;
     }
     CurrentPath[0]='/';
     CurrentPath[1]='\0';
     return VM_STATUS_SUCCESS;
     }
     
   else if(path[0]=='.'&&path[1]=='\0'){  
     return VM_STATUS_SUCCESS; 
     }
       else if(path[0]=='.'&&path[1]=='.'&&path[2]=='\0'&&CurrentPathRecord.size()>1){
       
       int num=0;
       int pathnum=0;
       while(CurrentPath[pathnum]!='\0') pathnum++;
       while((CurrentPathRecord[CurrentPathRecord.size()-1]->DDirEntry->insideDirEntry->DShortFileName[num])!=' ') num++;
       
       CurrentPath[pathnum-num]='\0';
       
       
       CurrentPathRecord.pop_back();
       CurrentPathNodeNum--;
       
       

       return VM_STATUS_SUCCESS;
     }

     
     else{
       //if(path[0]=='/') path++;
           
 
       if(!IsInDir(path,CurrentPathRecord[CurrentPathNodeNum-1])) return VM_STATUS_FAILURE;
       else 
       return VM_STATUS_SUCCESS;
     }
       
  }
}
bool IsInDir(const char * path, CurrentPathNode* node){
   
   
       int q =0;
       while(path[q]!='\0') q++;
       
           char s[VM_FILE_SYSTEM_MAX_PATH];
           strcpy(s,path);
           int p = 0;
          for(p=0;p<q;p++)
          { s[p] = s[p]-32;
          }
          
          for(p=q;p<8;p++){
            s[p]=' ';                      
          }
          s[p]='\0';

   
  unsigned int i;
  for(i=0;i<node->DAllEntry.size();i++){
  
    
           
// cout<<StringMatchN(node->DAllEntry[i]->insideDirEntry->DShortFileName,path,8)<<endl;
 // cout<<node->DAllEntry[i]->insideDirEntry->DShortFileName<<"   "<<path<<endl;
    if(StringMatchN(node->DAllEntry[i]->insideDirEntry->DShortFileName,s,8)!=1){}
    else {
      
    /*  char * abspath; 
      VMFileSystemGetAbsolutePath(abspath,CurrentPath,path);
      strcpy(CurrentPath,abspath);   
    */

         int q=0;
         int length=0;
         
         while(CurrentPath[length]!='\0') length++;
         
                  while(path[q]!='\0')
                  {
                    CurrentPath[length+q]=path[q];
                    q++;
                  
                  }
                  CurrentPath[q+length]='\0';
                   
         
     CurrentPathNode* newNode=new CurrentPathNode();
     newNode->DDirEntry=node->DAllEntry[i];
     newNode->DIR_FstClusLO=node->DAllEntry[i]->DIR_FstClusLO;               
     int offset =FirstDataSector*512+(newNode->DIR_FstClusLO-2)*2*512;
     uint8_t *Pointer;
     VMMemoryPoolAllocate(1,1024, (void **)&Pointer);
     readSector(512,offset,Pointer);
     int i,j;
     for (i=0;i<1024/32;i++){
       if(Pointer[32*i]==0x00) break;
       else if (Pointer[32*i]==0xe5) ;
       else {
          SVMDirectoryEntry* tempDirEntry=new SVMDirectoryEntry;
          FullDirEntry* tempFullDir=new FullDirEntry();        
          tempFullDir->DIR_FstClusLO=Pointer[26+32*i]|Pointer[27+32*i]<<8;      
             for( j=0;j<11;j++){       
                  tempDirEntry->DShortFileName[j]=Pointer[j+32*i];
              }
             tempDirEntry->DShortFileName[j]='\0';
             // cout<<"tempDirEntry->DIR_Name    "<<tempDirEntry->DIR_Name<<endl;
             tempDirEntry->DAttributes=Pointer[11+32*i];  
             tempDirEntry->DCreate.DDay=Pointer[16+32*i]&0x1f;
             tempDirEntry->DCreate.DMonth=(Pointer[16+32*i]|Pointer[17+32*i]<<8)>>5&0xf;
             tempDirEntry->DCreate.DYear=(Pointer[17+32*i]>>1)+1980;     
             tempDirEntry->DCreate.DSecond=Pointer[14+32*i]&0x1f;
             tempDirEntry->DCreate.DMinute=(Pointer[14+32*i]|Pointer[15+32*i]<<8)>>5&0x3f;
             tempDirEntry->DCreate.DHour=Pointer[15+32*i]>>3;
             tempDirEntry->DAccess.DDay=Pointer[18+32*i]&0x1f;
             tempDirEntry->DAccess.DMonth=(Pointer[18+32*i]|Pointer[19+32*i]<<8)>>5&0xf;
             tempDirEntry->DAccess.DYear=(Pointer[19+32*i]>>1)+1980;     
             tempDirEntry->DModify.DSecond=Pointer[22+32*i]&0x1f;
             tempDirEntry->DModify.DMinute=(Pointer[22+32*i]|Pointer[23+32*i]<<8)>>5&0x3f;
             tempDirEntry->DModify.DHour=Pointer[23+32*i]>>3; 
             tempDirEntry->DModify.DDay=Pointer[24+32*i]&0x1f;
             tempDirEntry->DModify.DMonth=(Pointer[24+32*i]|Pointer[25+32*i]<<8)>>5&0xf;
             tempDirEntry->DModify.DYear=(Pointer[25+32*i]>>1)+1980; 
             tempDirEntry->DSize=Pointer[28+32*i]|Pointer[29+32*i]<<8|Pointer[30+32*i]<<16|Pointer[31+32*i]<<24;         
             tempFullDir->insideDirEntry=tempDirEntry;             
             if(tempDirEntry->DAttributes!=0x0f){
               newNode->DAllEntry.push_back(tempFullDir);
             }       
       }   
     }     
     CurrentPathRecord.push_back(newNode);
     CurrentPathNodeNum++;
       VMMemoryPoolDeallocate(1,Pointer);  

      return true;
    }  
    
  }
  return false;
}
TVMStatus VMDirectoryClose(int dirdescriptor){
 return VM_STATUS_SUCCESS;

}

TVMStatus VMDirectoryRewind(int dirdescriptor){

 VMAllOpenDir[dirdescriptor]->pointerToDir=0;
 return VM_STATUS_SUCCESS;
}


int StringMatchN(const char *left, const char *right, int n){
    while(*left && *right && n){
        if(*left != *right){
            return 0;    
        }
        left++;
        right++;
        n--;
    }
    if(0 == n){
        return 1;    
    }
    return (*left == *right);
}

 
TVMStatus VMFileOpen(const char *filename, int flags, int mode,int *filedescriptor){
 if(filename==NULL||filedescriptor==NULL)  return  VM_STATUS_ERROR_INVALID_PARAMETER;
 else{
        int namelength=0;
        
        while(filename[namelength]!='\0') namelength++;
        
         
        char front[9]="0";
        char exten[4]="0"; 
        char combname[13];  
        int counter=0;
        int i=0;
        int j=0;
        while(filename[counter]!='.'&&counter!=namelength) 
        {
    
         if('a'<=filename[counter]||filename[counter]<='z')
          front[i]=filename[counter]-32;
         else
          front[i]=filename[counter];
          counter++;
          i++;
        }
        if(counter==namelength){
         for(;i<8;i++) front[i]=' ';
         for(;j<3;j++) exten[j]=' ';
         front[8]='\0';
         exten[3]='\0';
         strcpy(combname,front);
         strcpy(combname+8,exten);
        }
        else{
        counter++;
        while(filename[counter]!='\0')
        {
          if('a'<=filename[counter]||filename[counter]<='z')
            exten[j]=filename[counter]-32;
          else
            exten[j]=filename[counter];
          counter++;
          j++;
        }
        for(;i<8;i++) front[i]=' ';
        for(;j<3;j++) exten[j]=' ';
        front[8]='\0';
        exten[3]='\0';
        strcpy(combname,front);
        strcpy(combname+8,exten);
        }


  unsigned int num;
  for(num=0;num<CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry.size();num++){   
    if(StringMatchN(CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[num]->insideDirEntry->DShortFileName,combname,11)!=1);
    else{
         *filedescriptor=3+VMAllFileOpen.size();
         FileOpen* tempFile=new FileOpen();
         tempFile->num=num;
         tempFile->clusterCounter=0;
         tempFile->halfflag=0;
         tempFile->clusterNum=CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[num]->insideDirEntry->DSize/(512*FatBootSector->SecPerClus)+1;
         tempFile->clusterord=CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[num]->DIR_FstClusLO;
         
         VMAllFileOpen.push_back(tempFile);
         
         
         return  VM_STATUS_SUCCESS;       
    }
   
  } 
   return   VM_STATUS_FAILURE;
 }    
}

TVMStatus VMFileRead2(int filedescriptor, void *data, int *length){
   
   if(data==NULL||length==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
   else {   

              
              unsigned int num=VMAllFileOpen[filedescriptor-3]->num;
              unsigned int filesize= CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[num]->insideDirEntry->DSize;

              unsigned int fileleft=filesize-1024*(VMAllFileOpen[filedescriptor-3]->clusterCounter)-512*(VMAllFileOpen[filedescriptor-3]->halfflag);
             //locate the first cluster        
              
              if((VMAllFileOpen[filedescriptor-3]->clusterCounter)<(VMAllFileOpen[filedescriptor-3]->clusterNum)){
                  int memorysize=0;
                  
                  if(fileleft>=512) {memorysize=512;*length=1024;}
                  else{ 
                    memorysize=fileleft;
                    *length=1024;
                  }
         
                  int offset=FirstDataSector*512+(VMAllFileOpen[filedescriptor-3]->clusterord-2)*1024+512*(VMAllFileOpen[filedescriptor-3]->halfflag);
                  uint8_t * Pointer;       
                  VMMemoryPoolAllocate(1, memorysize, (void **)&Pointer);
                  readSector(memorysize,offset,Pointer);
                  
                 strncpy((char *)data,(char *)Pointer,memorysize);
                 cout<<(char*)Pointer<<endl;
                  VMMemoryPoolDeallocate(1,Pointer);

                  if(VMAllFileOpen[filedescriptor-3]->halfflag==0) VMAllFileOpen[filedescriptor-3]->halfflag=1;
                  else {
                    VMAllFileOpen[filedescriptor-3]->halfflag=0;
                    VMAllFileOpen[filedescriptor-3]->clusterCounter++;
                    VMAllFileOpen[filedescriptor-3]->clusterord=VMAllNextClusters[VMAllFileOpen[filedescriptor-3]->clusterord-2]; 
                  }
                  return VM_STATUS_SUCCESS;
                  
               }
               
              return    VM_STATUS_FAILURE;
  }  
                      
}
       




TVMStatus VMDirectoryCreate(const char *dirname){
  if(dirname==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
  else {
     FullDirEntry *tempEntry= new FullDirEntry();
     //tempEntry->insideDirEntry->DShortFileName="\0";
     int namelength=0;
     while(dirname[namelength]!='\0')      namelength++;
      tempEntry->insideDirEntry=new SVMDirectoryEntry;
     
     
     int counter=0;
     for(counter=0;counter<namelength;counter++){
        //cout<<counter<<" " <<namelength<<endl;
        if(dirname[counter]>='a'&&dirname[counter]<='z') tempEntry->insideDirEntry->DShortFileName[counter]=dirname[counter]-32;
        else  tempEntry->insideDirEntry->DShortFileName[counter]=dirname[counter];
        
        
       
     }
     
     //tempEntry->insideDirEntry->DShortFileName[counter]='\0';

    //strcpy(tempEntry->insideDirEntry->DShortFileName,dirname);
 
     tempEntry->insideDirEntry->DSize=0x00;
     tempEntry->insideDirEntry->DAttributes=16;
     tempEntry->insideDirEntry->DCreate=getCurrentTime();
     tempEntry->insideDirEntry->DModify=getCurrentTime();
     tempEntry->DIR_FstClusLO=getFirstFreeCluster();
      
     CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry.push_back(tempEntry);
     VMDirectoryRewind(VMAllOpenDir.size()-1);
     
     if(CurrentPathRecord.size()==1){
        
        int baseoffset=FirstRootSector*512;
        int bytes=(FirstDataSector-FirstRootSector)*512;
        uint8_t * Pointer;
        uint8_t * Pointer2;
     

        VMMemoryPoolAllocate(1, bytes, (void **)&Pointer);
        
        VMMemoryPoolAllocate(1, 32, (void **)&Pointer2);
        
  
        readSector(bytes,baseoffset,Pointer);
        int i;
        for(i=0;i<FatBootSector->RootEntCnt;i++){
          if( Pointer[32*i]==0x00 ){
  
            char content[32];
            int offset=baseoffset+32*i;
           // cout<<offset<<endl;
            int j=0;
            for(j=0;j<11;j++){
              if(j<namelength){
                
                if(dirname[j]>='a'&&dirname[j]<='z') content[j]=dirname[j]-32;
                else  content[j]=dirname[j];
              }
              else  content[j]=' ';
            }
            content[11]=16;
            for(j=12;j<22;j++)  content[j]=' '; 
            int year;
            char month,day,hour,min,sec;
            year=tempEntry->insideDirEntry->DModify.DYear-1980;
            month=tempEntry->insideDirEntry->DModify.DMonth;
            day=tempEntry->insideDirEntry->DModify.DDay;
            hour=tempEntry->insideDirEntry->DModify.DHour;
            min=tempEntry->insideDirEntry->DModify.DMinute;
            sec=tempEntry->insideDirEntry->DModify.DSecond;
            content[22]=(min&0x07)<<5|sec;
            j++;
            content[23]=(min&0x38)>>3|hour<<3;
            j++;
            content[24]=(month&0x07)<<5|day;
            j++;
            content[25]=month>>3|year<<1;
            for(j=26;j<28;j++)  content[j]=' '; 
            content[28]=0x00;
            content[29]=0x00;
            content[30]=0x00;
            
            content[31]=0x00;
            
            //cout<<content<<endl;
            //cout<<baseoffset<<endl;
            int m=0;
            for(m=0;m<32;m++){
            Pointer2[m]=content[m];
            
            }
           writeToFile(Pointer2,offset,32);
           
            return VM_STATUS_SUCCESS;             
          }
            
        }
       VMMemoryPoolDeallocate(1,Pointer);
       
       VMMemoryPoolDeallocate(1,Pointer2);
     }
        
     
     
    return VM_STATUS_SUCCESS;
  }
  
}
SVMDateTime getCurrentTime(){
  
  SVMDateTime currentTime;
   struct tm *local; 
   time_t t; 
   t=time(NULL); 

   local=localtime(&t);
   currentTime.DSecond=local->tm_sec/2;
   currentTime.DMinute=local->tm_min;
   currentTime.DHour=local->tm_hour;
   currentTime.DDay=local->tm_mday;
   currentTime.DMonth=local->tm_mon+1;
   currentTime.DYear=local->tm_year+1900;
return currentTime;
}
int getFirstFreeCluster(){

   int num=VMAllNextClusters.size();
   int i;
   for(i=0;i<num;i++){
     if(VMAllNextClusters[i]==0x0000){
       
       VMAllNextClusters[i]=0xffff;
       break;  
     }
   
   }
  
   return i;

}
void writeToFile(uint8_t *content,int offset,int length){
             void* calldata;

            calldata=(void*)VMAllThreads[1];
            
            TMachineFileCallback callback;
            callback=FileStaticCallback;
            
            //open
            MachineFileOpen("fat.ima", O_RDWR, 0644, callback, calldata);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMAllThreads[1]->DState=VM_THREAD_STATE_WAITING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[0]->DContext));
 
            int filedescriptor=VMAllThreads[1]->DFileReturn;

            //seek
            MachineFileSeek(filedescriptor, offset,  0, callback, calldata);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            
            //write
            MachineFileWrite(filedescriptor, content, length, callback, calldata);
            VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
            VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
            VMReadyQueueRemove(VMCurrentThreadID);
            MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
            
            //
           MachineFileClose(filedescriptor, callback, calldata);
           VMCurrentThreadID=VMReadyThreads[GetHighestPriority()].front()->DThreadID;
           VMReadyThreads[GetHighestPriority()].front()->DState=VM_THREAD_STATE_RUNNING;
           VMReadyQueueRemove(VMCurrentThreadID);
           MachineContextSwitch(&(VMAllThreads[1]->DContext),&(VMAllThreads[VMCurrentThreadID]->DContext));
}

TVMStatus VMDirectoryUnlink(const char *path){
  if (path==NULL) return VM_STATUS_ERROR_INVALID_PARAMETER;
  else {
     int size =CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry.size();
     int i=0;
     for (i=0;i<size;i++){
     
     char path2[13];
     int p=0,q=0;
     while(path[p]!='\0') p++;
     for(q=0;q<p;q++){
                if(path[q]>='a'&&path[q]<='z') path2[q]=path[q]-32;
                else  path2[q]=path[q];
       
     }
      if(StringMatchN(CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry[i]->insideDirEntry->DShortFileName,path2,p)!=1) ;
      else{
         
          CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry.erase(CurrentPathRecord[CurrentPathRecord.size()-1]->DAllEntry.begin()+i);
          VMDirectoryRewind(VMAllOpenDir.size()-1);
          return VM_STATUS_SUCCESS;
          
      }
        
     
     }
       
     
  return VM_STATUS_FAILURE;
  
  }


}

#ifdef __cplusplus
}
#endif

