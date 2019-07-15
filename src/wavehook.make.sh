gcc -rdynamic -L"/usr/local/cuda/lib64/" -L"/usr/lib/nvidia-390/" -L"/usr/lib/nvidia-390/" -I/usr/local/cuda-9.1/include -I/usr/local/cuda-9.1/targets/x86_64-linux/include/ -lcuda -lcudart -lnvidia-ml -lc -ldl -lrt -fPIC -c wavehook.c util.c wavehook.nvml.c wavehook.curt.c wavehook.cuda.c
ld -lc -ldl -lrt -shared -shared wavehook.o util.o wavehook.nvml.o wavehook.curt.o wavehook.cuda.o -g -o wavehook.so

