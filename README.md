# nvidia gpu using memory restriction (wavehook)
## wavehook function restrict and isolate nVidia GPU memory.

step1. export LD_PRELOAD=/path/wavehook.so

step2. export GPU_FRACTION=0.01 ~ 1.0

step3. run tensorflow_program

This Program intercept function cuMemGetInfo, cuMemGetInfo_v2, cuDeviceGetName, and another mainly methods in nVidia Driver.
Please refer to source program ...

