/**
 * @file gd.h
 * @brief Implementation of gradient descent optimizer.
 * @author Hideaki Takahashi
 */

#pragma once
#include <random>

#include "type.h"

namespace gymbo {

/**
 * @brief Gradient Descent Optimizer for Symbolic Path Constraints
 *
 * The `GDOptimizer` class provides functionality to optimize symbolic path
 * constraints by using gradient descent. It aims to find parameter values that
 * satisfy the given path constraints, making them true or non-positive.
 *
 * @note This class assumes that symbolic expressions are represented by the
 * `Sym` type, and path constraints are a vector of `Sym` objects.
 *
 * @note The optimization process relies on the gradient information of the path
 * constraints. The optimizer iteratively updates the parameters until the
 * constraints are satisfied or a maximum number of epochs is reached.
 */
struct GDOptimizer {
    int num_epochs;    ///< Maximum number of optimization epochs.
    float lr;          ///< Learning rate for gradient descent.
    float eps;         ///< The smallest positive value.
    float param_low;   ///< Lower bound for parameter initialization.
    float param_high;  ///< Upper bound for parameter initialization.
    bool sign_grad;    ///< If true, use sign gradient descent. Otherwise, use
                       ///< standard gradient descent. (default true).
    bool init_param_uniform_int;  ///< Flag indicating whether initial parameter
                                  ///< values are drawn from the uniform int
                                  ///< distribution or uniform real distribution
                                  ///< (default true).
    bool contain_randomized_vars;  ///< If true, use aeval and agrad. Otherwise,
                                   ///< use eval and grad.
    int seed;          ///< Random seed for initializing parameter values.
    int num_used_itr;  ///< Number of used iterations during optimization.

    /**
     * @brief Constructor for GDOptimizer.
     *
     * @param num_epochs Maximum number of optimization epochs (default: 100).
     * @param lr Learning rate for gradient descent (default: 1).
     * @param eps The smallest positive value of the target type
     * (default: 1.0).
     * @param param_low Lower bound for parameter initialization (default: -10).
     * @param param_high Upper bound for parameter initialization (default: 10).
     * @param sign_grad If true, use sign gradient descent. Otherwise, use
     * standard gradient descent. (default true).
     * @param init_param_uniform_int Flag indicating whether initial parameter
     * values are drawn from the uniform int distribution or uniform real
     * distribution (default true).
     * @param seed Random seed for initializing parameter values (default: 42).
     */
    GDOptimizer(int num_epochs = 100, float lr = 1.0, float eps = 1.0,
                float param_low = -10.0, float param_high = 10.0,
                bool sign_grad = true, bool init_param_uniform_int = true,
                int seed = 42)
        : num_epochs(num_epochs),
          lr(lr),
          eps(eps),
          param_low(param_low),
          param_high(param_high),
          sign_grad(sign_grad),
          init_param_uniform_int(init_param_uniform_int),
          seed(seed),
          num_used_itr(0) {}

    /**
     * @brief Evaluate if path constraints are satisfied for given parameters.
     *
     * This function checks if the given path constraints are satisfied
     * (non-positive) for the provided parameter values.
     *
     * @param path_constraints Vector of symbolic path constraints.
     * @param params Map of parameter values.
     * @return `true` if all constraints are satisfied; otherwise, `false`.
     */
    bool eval(std::vector<Sym> &path_constraints,
              std::unordered_map<int, float> params) {
        bool result = true;
        for (int i = 0; i < path_constraints.size(); i++) {
            result = result && (path_constraints[i].eval(params, eps) <= 0.0f);
        }
        return result;
    }

    /**
     * @brief Solve path constraints using gradient descent optimization.
     *
     * This function attempts to find parameter values that satisfy the given
     * path constraints by using gradient descent optimization. It iteratively
     * updates the parameters until the constraints are satisfied or the maximum
     * number of epochs is reached.
     *
     * @param path_constraints Vector of symbolic path constraints.
     * @param params Map of parameter values (will be modified during
     * optimization).
     * @param is_init_params_const Flag indicating whether initial parameter
     * values are constant.
     * @return `true` if the constraints are satisfied after optimization;
     * otherwise, `false`.
     */
    bool solve(std::vector<Sym> &path_constraints,
               std::unordered_map<int, float> &params,
               bool is_init_params_const = true) {
        if (path_constraints.size() == 0) {
            return true;
        }

        std::mt19937 gen(seed);
        std::uniform_int_distribution<> idist(param_low, param_high);
        std::uniform_real_distribution<float> rdist(param_low, param_high);

        std::unordered_map<int, bool> is_const;
        std::unordered_set<int> unique_var_ids;

        for (int i = 0; i < path_constraints.size(); i++) {
            path_constraints[i].gather_var_ids(unique_var_ids);
        }

        for (int i : unique_var_ids) {
            if (params.find(i) == params.end()) {
                if (init_param_uniform_int) {
                    params.emplace(std::make_pair(i, idist(gen)));
                } else {
                    params.emplace(std::make_pair(i, rdist(gen)));
                }
                is_const.emplace(std::make_pair(i, false));
            } else {
                is_const.emplace(std::make_pair(i, is_init_params_const));
            }
        }

        int itr = 0;
        bool is_sat = eval(path_constraints, params);
        bool is_converge = false;

        while ((!is_sat) && (!is_converge) && (itr < num_epochs)) {
            Grad grads = Grad({});
            for (int i = 0; i < path_constraints.size(); i++) {
                if (path_constraints[i].eval(params, eps) > 0.0f) {
                    grads = grads + path_constraints[i].grad(params, eps);
                }
            }
            is_converge = true;
            for (auto &g : grads.val) {
                if (!is_const.at(g.first)) {
                    if (g.second != 0.0f) {
                        is_converge = false;
                    }
                    if (!sign_grad) {
                        params.at(g.first) -= lr * g.second;
                    } else {
                        float sign = 0.0f;
                        if (g.second > 0.0f) {
                            sign = 1.0;
                        } else if (g.second < 0.0f) {
                            sign = -1.0f;
                        }
                        params.at(g.first) -= lr * sign;
                    }
                }
            }
            is_sat = eval(path_constraints, params);
            itr++;
            num_used_itr++;
        }
        return is_sat;
    }
};
}  // namespace gymbo
