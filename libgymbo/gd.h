#pragma once
#include "type.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gymbo {
struct GDOptimizer {

  int num_epochs, lr;

  GDOptimizer(int num_epochs = 100, int lr = 1)
      : num_epochs(num_epochs), lr(lr) {}

  bool eval(std::vector<Sym> &path_constraints,
            std::unordered_map<int, int> params) {
    bool result = true;
    for (int i = 0; i < path_constraints.size(); i++) {
      result = result && (path_constraints[i].eval(params) <= 0);
    }
    return result;
  }

  bool solve(std::vector<Sym> &path_constraints,
             std::unordered_map<int, int> &params,
             bool is_init_params_const = true) {
    if (path_constraints.size() == 0) {
      return true;
    }

    std::unordered_map<int, int> is_const;
    std::unordered_set<int> unique_var_ids;

    for (int i = 0; i < path_constraints.size(); i++) {
      path_constraints[i].gather_var_ids(unique_var_ids);
    }

    for (int i : unique_var_ids) {
      if (params.find(i) == params.end()) {
        params.emplace(std::make_pair(i, 0));
        is_const.emplace(std::make_pair(i, false));
      } else {
        is_const.emplace(std::make_pair(i, is_init_params_const));
      }
    }

    int itr = 0;
    bool is_sat = eval(path_constraints, params);

    while ((!is_sat) && (itr < num_epochs)) {
      Grad grads = Grad({});
      for (int i = 0; i < path_constraints.size(); i++) {
        if (path_constraints[i].eval(params) > 0) {
          grads = grads + path_constraints[i].grad(params);
          for (auto &g : grads.val) {
            if (!is_const.at(g.first)) {
              params.at(g.first) -= lr * g.second;
            }
          }
        }
      }
      is_sat = eval(path_constraints, params);
      itr++;
    }

    return is_sat;
  }
};
} // namespace gymbo
