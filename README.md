# wavehook-for-gpu-memory-restriction
wavehook for gpu memory restriction using nVidia GPU

step1. export LD_PRELOAD=/path/wavehook.so

step2. export GPU_FRACTION=0.01 ~ 1.0

step3. run tensorflow_program

This Program intercept mainly function named cuMemGetInfo, cuMemGetInfo_v2, cuDeviceGetName in nVidia Driver.
Please refer to source program ...

