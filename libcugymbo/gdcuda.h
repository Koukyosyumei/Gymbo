#pragma once
#ifndef CUDA_VEC_DOUBLE_H
#define CUDA_VEC_DOUBLE_H

#include "../libgymbo/gd.h"

void vecDouble(int* hIn, int* hOut, const int n);

namespace gymbo {

struct GDOptimizerCUDA : public Optimizer {
    using Optimizer::Optimizer;

    bool solve(std::vector<Sym>& path_constraints,
               std::unordered_map<int, float>& params,
               bool is_init_params_const = true) override;
};

}  // namespace gymbo

#endif /* CUDA_VEC_DOUBLE_H */
