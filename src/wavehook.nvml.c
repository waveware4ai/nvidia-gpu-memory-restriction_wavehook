#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include "wavehook.h"

#include <nvml.h>


#define NVML_CALL(func, ...)  {  nvmlReturn_t DECLDIR status = func(__VA_ARGS__); }
extern void*libnvml;
extern void*native_funload(void*lib, char*fun);

extern int dbg; 

#undef nvmlDeviceGetMemoryInfo
nvmlReturn_t DECLDIR nvmlDeviceGetMemoryInfo(nvmlDevice_t device, nvmlMemory_t *memory)
{
    if (dbg) printf ("[INF] nvml func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libnvml, (char*)__func__);
    NVML_CALL(fp, device, memory);
    
    size_t free  = memory->free;
    size_t total = memory->total;
    MemIsolator(&free, &total);
    memory->free = free;
    memory->total = total;
    memory->used = total - free;
    
    return 0;
}

#undef nvmlDeviceGetPciInfo_v2
nvmlReturn_t DECLDIR nvmlDeviceGetPciInfo_v2(nvmlDevice_t device, nvmlPciInfo_t *pci)
{
    if (dbg) printf ("[INF] nvml func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libnvml, (char*)__func__);
    NVML_CALL(fp, device, pci);
    return 0;
}

#undef nvmlDeviceGetName
nvmlReturn_t DECLDIR nvmlDeviceGetName(nvmlDevice_t device, char *name, unsigned int length)
{
    if (dbg) printf ("[INF] nvml func : %s\n", __func__);
    int64_t (*fp)() = native_funload(libnvml, (char*)__func__);
    NVML_CALL(fp, device, name, length);
    DevNameIsolator(name);
    return 0;
}



