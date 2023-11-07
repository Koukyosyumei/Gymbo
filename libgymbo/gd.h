#pragma once
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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

  int num_epochs, lr, seed, param_low, param_high;
  bool sign_grad;

  /**
   * @brief Constructor for GDOptimizer.
   *
   * @param num_epochs Maximum number of optimization epochs (default: 100).
   * @param lr Learning rate for gradient descent (default: 1).
   * @param param_low Lower bound for parameter initialization (default: -10).
   * @param param_high Upper bound for parameter initialization (default: 10).
   * @param sign_grad If true, use sign gradient descent. Otherwise, use
   * standard gradient descent. (default true).
   * @param seed Random seed for initializing parameter values (default: 42).
   */
  GDOptimizer(int num_epochs = 100, int lr = 1, int param_low = -10,
              int param_high = 10, bool sign_grad = true, int seed = 42)
      : num_epochs(num_epochs), lr(std::max(1, lr)), param_low(param_low),
        param_high(param_high), sign_grad(sign_grad), seed(seed) {}

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
            std::unordered_map<std::string, bool> &assignments,
            std::unordered_map<int, int> &params) {
    bool result = true;
    for (int i = 0; i < path_constraints.size(); i++) {
      if (assignments[path_constraints[i].toString()]) {
        result = result && (path_constraints[i].eval(params) <= 0);
      } else {
        result = result && (path_constraints[i].eval(params) <= 0);
      }
    }
    return result;
  }

  /**
   * @brief Solve path constraints using gradient descent optimization.
   *
   * This function attempts to find parameter values that satisfy the given path
   * constraints by using gradient descent optimization. It iteratively updates
   * the parameters until the constraints are satisfied or the maximum number of
   * epochs is reached.
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
             std::unordered_map<std::string, bool> &assignments,
             std::unordered_map<int, int> &params,
             bool is_init_params_const = true) {
    if (path_constraints.size() == 0) {
      return true;
    }

    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(param_low, param_high);

    std::unordered_map<int, int> is_const;
    std::unordered_set<int> unique_var_ids;

    for (int i = 0; i < path_constraints.size(); i++) {
      path_constraints[i].gather_var_ids(unique_var_ids);
    }

    for (int i : unique_var_ids) {
      if (params.find(i) == params.end()) {
        params.emplace(std::make_pair(i, dist(gen)));
        is_const.emplace(std::make_pair(i, false));
      } else {
        is_const.emplace(std::make_pair(i, is_init_params_const));
      }
    }

    int itr = 0;
    bool is_sat = eval(path_constraints, assignments, params);

    while ((!is_sat) && (itr < num_epochs)) {
      Grad grads = Grad({});
      for (int i = 0; i < path_constraints.size(); i++) {
        bool ass = assignments[path_constraints[i].toString()];
        bool stop = false;
        if (ass) {
          stop = path_constraints[i].eval(params) <= 0;
        } else {
          stop = path_constraints[i].eval(params) > 0;
        }
        if (!stop) {
          grads = grads + path_constraints[i].grad(params);
          for (auto &g : grads.val) {
            if (!is_const.at(g.first)) {
              if (!sign_grad) {
                if (ass) {
                  params.at(g.first) -= lr * g.second;
                } else {
                  params.at(g.first) += lr * g.second;
                }
              } else {
                int sign = 0;
                if (g.second > 0) {
                  sign = 1;
                } else if (g.second < 0) {
                  sign = -1;
                }
                if (ass) {
                  params.at(g.first) -= lr * sign;
                } else {
                  params.at(g.first) += lr * sign;
                }
              }
            }
          }
        }
      }
      is_sat = eval(path_constraints, assignments, params);
      itr++;
    }
    return is_sat;
  }
};
} // namespace gymbo
