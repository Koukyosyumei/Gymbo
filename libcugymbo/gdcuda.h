#pragma once
#ifndef CUDA_VEC_DOUBLE_H
#define CUDA_VEC_DOUBLE_H

#include "../libgymbo/gd.h"

void vecDouble(int *hIn, int *hOut, const int n);

namespace gymbo {

struct GDOptimizerCUDA : public GDOptimizer {
    using GDOptimizer::GDOptimizer;
};

}  // namespace gymbo

#endif /* CUDA_VEC_DOUBLE_H */
