#include "cuda_runtime.h"
#include "CudaVecDouble.h"

__global__ void kernel_vecDouble(int *in, int *out, const int n)
{
    int i = threadIdx.x;
    if (i < n) {
        out[i] = in[i] * 2;
    }
}

void vecDouble(int *hIn, int *hOut, const int n)
{
    int *dIn;
    int *dOut;
    cudaMallocHost((void**)&dIn, n * sizeof(int));
    cudaMallocHost((void**)&dOut, n * sizeof(int));
    cudaMemcpy(dIn, hIn, n * sizeof(int), cudaMemcpyHostToDevice);

    kernel_vecDouble<<<1, n>>>(dIn, dOut, n);
    cudaDeviceSynchronize();

    cudaMemcpy(hOut, dOut, n * sizeof(int), cudaMemcpyDeviceToHost);
    cudaFree(dIn);
    cudaFree(dOut);
}
