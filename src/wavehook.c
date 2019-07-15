//https://docs.nvidia.com/cuda/cuda-driver-api/group__CUDA__MEM.html
//https://docs.nvidia.com/deploy/nvml-api/structnvmlBAR1Memory__t.html#structnvmlBAR1Memory__t
//https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1g52c5036ce0db5dc56ad61f284500f3eb
//https://www.cs.cmu.edu/afs/cs/academic/class/15668-s11/www/cuda-doc/html/group__CUDART__MEMORY_gd5d6772f4b2f3355078ecd6059e6aa74.html
//https://github.com/HongjianLi/cuda/blob/master/deviceQuery/deviceQuery.cpp

    
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include "wavehook.h"

void*libcuda = NULL;
void*libcurt = NULL;
void*libnvml = NULL;
void*native_libload(char*so)
{
  //print_backtrace();
  //print_trace();
  //print_walk_through();
    void*lib = dlopen(so, RTLD_NOW); //RTLD_NOW, RTLD_LAZY, RTLD_GLOBAL
    if (NULL == lib) 
    {
        printf("[ERR] cannot open %s ...\n", so);
    }
    return lib; 
}

void*native_funload(void*lib, char*fun)
{
    void*ptr = dlsym(lib, fun); //RTLD_NEXT, libcuda
    if (NULL == ptr) 
    {
        printf("[ERR] cannot load %s ...\n", fun);
    } 
    return ptr;
}

int    dbg = 0; 
char  *usr = NULL;
int    pid = -1;
float  usr_fraction = 1;
float  prs_fraction = 1;

#include <nvml.h>

#define NVML_CALL(func, ...)  {  nvmlReturn_t DECLDIR status = func(__VA_ARGS__); }


size_t getGPUMemUsageByUser()
{
    // id -u username : user name to user id
    // ps -u -p 1234 ; pid's user name 
    // ps f -o user,pgrp,pid,pcpu,pmem,start,time,command
    
/*
zookim@w011:~/wavehook.test$ ps f -o pid
  PID
30489
27974
21112
19200

zookim@w011:~/wavehook.test$ nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits
27658, python, 11614 MiB

*/    

    int pls[32]; // pid list
    int mls[32]; // mem list
    getpidbyusr(usr, 32, pls);
    getmembypid(32, pls, mls);
    size_t use = 0;
    for (int i = 0; i < 32; i++)
    { 
        if (pls[i] < 0) break; 
        printf ("pid [%d] == [%d]\n", pls[i], mls[i]);
        if (0 < mls[i]) use += mls[i] * 1024 * 1024;
    } 
    return use;
}

int MemIsolator(size_t*free, size_t*total)
{
    size_t memFree  = *free;
    size_t memUsed  = 0;
    size_t memTotal = *total;
    size_t memAlloc = 0;

    if (usr_fraction == 1) 
    { //full access to resource
        memTotal = (int64_t)((float)(memTotal)* 1);
        memUsed  = memTotal - memFree;
        memAlloc = (int64_t)((float)(memTotal)* prs_fraction) + 0x2000000; //reserved area : 0x2000000 : 32MB,    
        if (memFree < memAlloc) memAlloc = memFree;

        *free  = memAlloc;
        *total = memAlloc;
    }
    else
    { //user isolation access to resource
        memTotal = (int64_t)((float)(memTotal)* usr_fraction);
        memUsed  = getGPUMemUsageByUser(); //memTotal - memFree; // calcuration gpu memories for this user 
        memFree  = memTotal - memUsed;
        memAlloc = (int64_t)((float)(memTotal)* prs_fraction) + 0x2000000; //reserved area : 0x2000000 : 32MB,    
        if (memFree < memAlloc) memAlloc = memFree;

        *free  = memAlloc;
        *total = memAlloc;
    }   
    
    if (1)
    {
        printf("########################\n"); 
        printf("[INF] GPU free  %5zu MiB (%5zuB)\n", memFree  / 1024 / 1024, memFree);
        printf("[INF] GPU used  %5zu MiB (%5zuB)\n", memUsed  / 1024 / 1024, memUsed);
        printf("[INF] GPU total %5zu MiB (%5zuB)\n", memTotal / 1024 / 1024, memTotal);
        printf("[INF] GPU alloc %5zu MiB (%5zuB)\n", memAlloc / 1024 / 1024, memAlloc);
        printf("########################\n"); 
    }
        
    
/*
    if (0)
    {
        nvmlDevice_t device;
        int64_t (*f1)() = native_funload(libnvml, "nvmlDeviceGetHandleByIndex");
        NVML_CALL(f1, 0, &device);
        
        nvmlMemory_t mem;
        int64_t (*f2)() = native_funload(libnvml, "nvmlDeviceGetMemoryInfo");
        NVML_CALL(f2, device, &mem);
  
        nvmlUtilization_t util;
        int64_t (*f3)() = native_funload(libnvml, "nvmlDeviceGetUtilizationRates");
        NVML_CALL(f3, device, &util);

        unsigned int pa_count=0;
        unsigned int*pa=NULL;
        int64_t (*f4)() = native_funload(libnvml, "nvmlDeviceGetAccountingPids");
        NVML_CALL(f4, device, &pa_count, pa);
        
          printf("[INF] %d\n", pa_count);
        
        for(int j=0; j<pa_count; j++) 
        {
            nvmlAccountingStats_t stat;
            int64_t (*f5)() = native_funload(libnvml, "nvmlDeviceGetAccountingStats");
            NVML_CALL(f5, device, pa[j], &stat);
            
            // per process accounting (cumulative) statistics 
            printf("%4d %4d %4d %4d %6d\n", 0, pa[j], stat.gpuUtilization, stat.memoryUtilization, stat.maxMemoryUsage/1024/1024);
        }
    }*/      
}

int DevNameIsolator(char*name)
{
    char buf[64] = {0,};
    if (usr_fraction != 1) 
        sprintf(buf, "Isolated %s [Fraction(%1.1f%%/%1.1f%%)=%1.1f%%]", name, prs_fraction*100, usr_fraction*100, usr_fraction*prs_fraction*100);
    else
        sprintf(buf, "Isolated %s [Fraction=%1.1f%%]", name, prs_fraction*100);
    if (0) printf("%s\n", buf);
    strcpy(name, buf);    
}

///////////////////////////////////////////////////////////////////////////////

void handler(int signal) 
{
    printf("[ERR] signal %d exit ...\n", signal);
    exit(1);
}

void _init(void)
{// for global hook, put this to /etc/ld.so.preload !!!
    libcuda = native_libload("libcuda.so");    
    libcurt = native_libload("libcudart.so");   
    libnvml = native_libload("libnvidia-ml.so"); 

    usr = getusrbyuid(geteuid()); //getenv("USER");
    char did[16] = {0,}; // reserved, device id
    char val[16] = {0,};
    if (parse(PROFILE, usr, did, val)) usr_fraction = atof(val); 
    usr_fraction = MIN(1, MAX(0, usr_fraction));    

    char *mem = getenv("GPU_FRACTION"); // clamped value 0~1.0 
    if (mem == NULL) prs_fraction = 1; else prs_fraction = atof(mem);
    prs_fraction = MIN(1, MAX(0, prs_fraction));

    if (pid < 0)
    {
        printf("\n########################\n");
        printf("GPU Hook by @waveware ... %s(%d)\n", usr, geteuid());
        printf("GPU Allocation Clamped User    to %1.2f\n", usr_fraction);
        printf("GPU Allocation Clamped Context to %1.2f\n", prs_fraction);
        printf("########################\n\n");
        signal(SIGSEGV, handler);
        pid = geteuid();
    }
}
