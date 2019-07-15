#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include "wavehook.h"

//#include <cuda_runtime.h>
#include <cuda_runtime.h>


#define CURT_CALL(func, ...)  {  cudaError_t CUDARTAPI status = func(__VA_ARGS__); }
extern void*libcurt;
extern void*native_funload(void*lib, char*fun);

extern int dbg; 

#undef cudaMemGetInfo
extern __host__ cudaError_t CUDARTAPI cudaMemGetInfo(size_t *free, size_t *total)
{
    if (dbg) printf ("[INF] curt func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcurt, (char*)__func__);
    CURT_CALL(fp, free, total);
    MemIsolator(free, total);
    return 0;
}

#undef cudaGetDeviceProperties
extern __host__ __cudart_builtin__ cudaError_t CUDARTAPI cudaGetDeviceProperties(struct cudaDeviceProp *prop, int device)
{
    if (dbg) printf ("[INF] curt func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libcurt, (char*)__func__);
    CURT_CALL(fp, prop, device);
    DevNameIsolator(prop->name);
    return 0;
}

