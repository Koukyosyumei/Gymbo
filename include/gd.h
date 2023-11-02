#pragma once
#include "type.h"
#include <unordered_map>
#include <utility>

struct GDOptimizer {

  int num_epochs, lr;

  GDOptimizer(int num_epochs = 100, int lr = 1)
      : num_epochs(num_epochs), lr(lr) {}

  int eval(std::vector<Sym> &path_constraints,
           std::unordered_map<int, int> params) {
    int result = 0;
    for (int i = 0; i < path_constraints.size(); i++) {
      result += path_constraints[i].eval(params);
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
    Grad grads = Grad({});

    for (int i = 0; i < path_constraints.size(); i++) {
      grads = grads + path_constraints[i].grad();
    }

    for (auto &g : grads.val) {
      if (params.find(g.first) == params.end()) {
        params.emplace(std::make_pair(g.first, 0));
        is_const.emplace(std::make_pair(g.first, false));
      } else {
        is_const.emplace(std::make_pair(g.first, is_init_params_const));
      }
    }

    int i = 0;
    int loss = eval(path_constraints, params);

    while (loss > 0 && i < num_epochs) {
      for (auto &g : grads.val) {
        if (!is_const.at(g.first)) {
          params.at(g.first) -= lr * g.second;
        }
      }
      loss = eval(path_constraints, params);
      i++;
    }

    return loss <= 0;
  }
};
