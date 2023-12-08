/**
 * @file smt.h
 * @brief Core implementation of gradient-based smt solver
 * @author Hideaki Takahashi
 */

#pragma once
#include "gd.h"
#include "sat.h"
namespace gymbo {

/**
 * @brief Initialize Parameter Values from Memory.
 *
 * This function initializes parameter values from memory, provided that
 * `ignore_memory` is set to false.
 *
 * @param params A map to store parameter values.
 * @param state The symbolic state of the program.
 * @param ignore_memory A flag indicating whether memory constraints should be
 * ignored.
 */
inline void initialize_params(std::unordered_map<int, float> &params,
                              SymState &state, bool ignore_memory) {
    params = {};
    if (!ignore_memory) {
        for (auto &p : state.mem) {
            params.emplace(std::make_pair(p.first, wordToFloat(p.second)));
        }
    }
}

/**
 * @brief SMT Solver with Unified Loss Function.
 *
 * @param is_sat Flag to store the result of satisfiability.
 * @param state The current symbolic state.
 * @param params The map from the variable IDs to the concrete values.
 * @param optimizer The gradient descent optimizer for parameter optimization.
 * @param max_num_trials The maximum number of trials for each gradient descent.
 * @param ignore_memory If set to true, constraints derived from memory will be
 * ignored.
 */
inline void smt_union_solver(bool &is_sat, SymState &state,
                             std::unordered_map<int, float> &params,
                             GDOptimizer &optimizer, int max_num_trials,
                             bool ignore_memory) {
    for (int j = 0; j < max_num_trials; j++) {
        is_sat = optimizer.solve(state.path_constraints, params);
        if (is_sat) {
            break;
        }
        optimizer.seed += 1;
        initialize_params(params, state, ignore_memory);
    }
}

/**
 * @brief SMT Solver using DPLL as its backend.
 *
 * @param is_sat Flag to store the result of satisfiability.
 * @param state The current symbolic state.
 * @param params The map from the variable IDs to the concrete values.
 * @param optimizer The gradient descent optimizer for parameter optimization.
 * @param max_num_trials The maximum number of trials for each gradient descent.
 * @param ignore_memory If set to true, constraints derived from memory will be
 * ignored.
 */
inline void smt_dpll_solver(bool &is_sat, SymState &state,
                            std::unordered_map<int, float> &params,
                            GDOptimizer &optimizer, int max_num_trials,
                            bool ignore_memory) {
    std::unordered_map<std::string, gymbo::Sym *> unique_terms_map;
    std::unordered_map<std::string, bool> assignments_map;
    std::shared_ptr<gymbosat::Expr> path_constraints_expr =
        gymbosat::pathconstraints2expr(state.path_constraints,
                                       unique_terms_map);

    while (satisfiableDPLL(path_constraints_expr, assignments_map)) {
        std::vector<Sym> new_constraints;
        for (auto &ass : assignments_map) {
            if (ass.second) {
                new_constraints.emplace_back(*unique_terms_map[ass.first]);
            } else {
                new_constraints.emplace_back(
                    Sym(SymType::SNot, unique_terms_map[ass.first]));
            }
        }

        for (int j = 0; j < max_num_trials; j++) {
            is_sat = optimizer.solve(new_constraints, params);
            if (is_sat) {
                break;
            }
            optimizer.seed += 1;
            initialize_params(params, state, ignore_memory);
        }

        if (is_sat) {
            break;
        }

        // add feedback
        std::shared_ptr<gymbosat::Expr> learnt_constraints =
            std::make_shared<gymbosat::Const>(false);
        for (auto &ass : assignments_map) {
            if (ass.second) {
                learnt_constraints = std::make_shared<gymbosat::Or>(
                    learnt_constraints,
                    std::make_shared<gymbosat::Not>(
                        std::make_shared<gymbosat::Var>(ass.first)));
            } else {
                learnt_constraints = std::make_shared<gymbosat::Or>(
                    learnt_constraints,
                    std::make_shared<gymbosat::Var>(ass.first));
            }
        }

        path_constraints_expr = std::make_shared<gymbosat::And>(
            path_constraints_expr, learnt_constraints);
    }
}

}  // namespace gymbo
