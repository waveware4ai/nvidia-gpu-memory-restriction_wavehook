#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include "wavehook.h"
 
#include <cuda.h>

#define __CUDA_API_VERSION_INTERNAL

#define CUDA_CALL(func, ...)  {  CUresult CUDAAPI status = func(__VA_ARGS__); }
extern void*libcuda;
extern void*native_funload(void*lib, char*fun);

extern int dbg; 


#undef cuMemGetInfo
CUresult CUDAAPI cuMemGetInfo(size_t *free, size_t *total)
{
    return cuMemGetInfo_v2(free, total);
}

#undef cuMemGetInfo_v2
CUresult CUDAAPI cuMemGetInfo_v2(size_t *free, size_t *total)
{
    if (dbg) printf ("[INF] cuda func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcuda, (char*)__func__);
    CUDA_CALL(fp, free, total);
    MemIsolator(free, total);
    return 0;
}

#undef cuDeviceGetName
CUresult CUDAAPI cuDeviceGetName(char *name, int len, CUdevice dev)
{
    if (dbg) printf ("[INF] cuda func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcuda, (char*)__func__);
    CUDA_CALL(fp, name, len, dev);
    DevNameIsolator(name);
    return 0;
}

#undef cuDeviceGetProperties
CUresult CUDAAPI cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
    if (dbg) printf ("[INF] cuda func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcuda, (char*)__func__);
    CUDA_CALL(fp, prop, dev);
    return 0;
}

#undef cuDeviceComputeCapability
CUresult CUDAAPI cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
    if (dbg) printf ("[INF] cuda func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcuda, (char*)__func__);
    CUDA_CALL(fp, major, minor, dev);
    return 0;
}

