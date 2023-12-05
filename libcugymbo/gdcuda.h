#pragma once
#include <cuda_runtime.h>

#include <vector>

#include "../libgymbo/gd.h"

namespace gymbo {

// CUDA kernel for gradient descent
__global__ void gradientDescentKernel(float* params, float* grads,
                                      int num_params, float lr,
                                      bool sign_grad) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    // Check bounds to avoid accessing out-of-bounds memory
    if (tid < num_params) {
        if (!sign_grad) {
            params[tid] -= lr * grads[tid];
        } else {
            float sign = (grads[tid] > 0.0f)
                             ? 1.0f
                             : ((grads[tid] < 0.0f) ? -1.0f : 0.0f);
            params[tid] -= lr * sign;
        }
    }
}

struct GDOptimizerCUDA : public GDOptimizer {
    using GDOptimizer::GDOptimizer;

    bool solve(std::vector<Sym>& path_constraints,
               std::unordered_map<int, float>& params,
               bool is_init_params_const = true) {
        // ... (unchanged)
        int num_params = params.size();

        std::vector<float> h_params;
        for (auto& p : params) {
            h_params.emplace_back(p.second);
        }

        // Allocate GPU memory for parameters and gradients
        float* d_params;
        float* d_grads;
        cudaMalloc((void**)&d_params, num_params * sizeof(float));
        cudaMalloc((void**)&d_grads, num_params * sizeof(float));

        // Copy parameters from host to device
        cudaMemcpy(d_params, h_params.data(), num_params * sizeof(float),
                   cudaMemcpyHostToDevice);

        int itr = 0;
        bool is_sat = eval(path_constraints, params);
        bool is_converge = false;

        while ((!is_sat) && (!is_converge) && (itr < num_epochs)) {
            // Calculate gradients on the GPU
            calculateGradientsKernel<<<num_blocks, block_size>>>(
                d_params, d_grads, num_params);

            // Copy gradients from device to host
            cudaMemcpy(h_grads.data(), d_grads, num_params * sizeof(float),
                       cudaMemcpyDeviceToHost);

            gradientDescentKernel<<<num_blocks, block_size>>>(
                d_params, d_grads, num_params, lr, sign_grad);

            // Copy updated parameters from host to device
            cudaMemcpy(d_params, h_params.data(), num_params * sizeof(float),
                       cudaMemcpyHostToDevice);

            is_sat = eval(path_constraints, params);
            itr++;
            num_used_itr++;
        }

        // Free GPU memory
        cudaFree(d_params);
        cudaFree(d_grads);

        return is_sat;
    }
};
}  // namespace gymbo
