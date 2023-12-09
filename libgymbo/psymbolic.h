/**
 * @file psymbolic.h
 * @brief Implementation of symbolic execution for randomized programs.
 * @author Hideaki Takahashi
 */

#pragma once
#include <unordered_map>
#include <unordered_set>

#include "gd.h"
#include "symbolic.h"
#include "type.h"
#include "utils.h"

namespace gymbo {

inline std::vector<std::vector<int>> cartesianProduct(
    const std::vector<std::vector<int>> &vectors) {
    std::vector<std::vector<int>> result;

    if (vectors.size() == 0) {
        return result;
    }

    for (int v : vectors[0]) {
        std::vector<int> tmp = {v};
        result.emplace_back(tmp);
    }

    for (int i = 1; i < vectors.size(); ++i) {
        std::vector<std::vector<int>> tempResult;
        for (const auto &element1 : result) {
            for (const auto &element2 : vectors[i]) {
                std::vector<int> tempElement = element1;
                tempElement.push_back(element2);
                tempResult.push_back(tempElement);
            }
        }
        result = tempResult;
    }

    return result;
}

inline int pbranch(std::unordered_map<int, DiscreteDist> &var2dist,
                   std::vector<std::vector<int>> &D, SymState state,
                   GDOptimizer &optimizer, int max_num_trials, bool use_dpll,
                   std::unordered_map<int, float> params,
                   std::unordered_set<int> &unique_var_ids) {
    for (auto &vd : var2dist) {
        state.mem.emplace(vd.first, FloatToWord(0));
    }

    int total_num_pvar_combinations = D.size();

    bool ignore_memory = false;
    int num_sat = 0;

    printf("total_num_pvac %d\n", total_num_pvar_combinations);

    for (int i = 0; i < total_num_pvar_combinations; i++) {
        bool is_sat = false;
        int j = 0;
        for (auto &vd : var2dist) {
            state.mem[vd.first] = FloatToWord(D[i][j]);
            j++;
        }

        initialize_params(params, state, ignore_memory);

        if (use_dpll) {
            smt_dpll_solver(is_sat, state, params, optimizer, max_num_trials,
                            ignore_memory);
        } else {
            smt_union_solver(is_sat, state, params, optimizer, max_num_trials,
                             ignore_memory);
        }

        if (is_sat) {
            num_sat++;
        }
    }

    return num_sat;
}

inline Trace run_pgymbo(Prog &prog,
                        std::unordered_map<int, DiscreteDist> &var2dist,
                        std::vector<std::vector<int>> &D,
                        GDOptimizer &optimizer, SymState &state,
                        std::unordered_set<int> &target_pcs,
                        PathConstraintsTable &constraints_cache, int maxDepth,
                        int &maxSAT, int &maxUNSAT, int max_num_trials,
                        bool ignore_memory, bool use_dpll, int verbose_level,
                        bool return_trace = false) {
    int pc = state.pc;
    if (verbose_level >= -1) {
        printf("pc: %d, ", pc);
        prog[pc].print();
        if (verbose_level >= 2) {
            state.print();
        }
    }

    bool is_target = ((target_pcs.size() == 0) ||
                      (target_pcs.find(-1) != target_pcs.end()) ||
                      (target_pcs.find(pc) != target_pcs.end()));
    bool is_sat = true;

    if (state.path_constraints.size() != 0 && is_target) {
        std::string constraints_str = state.toString();

        std::unordered_map<int, float> params = {};
        initialize_params(params, state, ignore_memory);

        bool is_unknown_path_constraint = true;

        if (constraints_cache.find(constraints_str) !=
            constraints_cache.end()) {
            is_sat = constraints_cache[constraints_str].first;
            params = constraints_cache[constraints_str].second;
            is_unknown_path_constraint = false;
        } else {
            bool is_contain_prob_var = false;
            std::unordered_set<int> unique_var_ids;
            for (int i = 0; i < state.path_constraints.size(); i++) {
                state.path_constraints[i].gather_var_ids(unique_var_ids);
            }
            for (int i : unique_var_ids) {
                if (var2dist.find(i) != var2dist.end()) {
                    is_contain_prob_var = true;
                    break;
                }
            }

            if (is_contain_prob_var) {
                // call probabilistic branch algorithm
                int total_num_sat_comb =
                    pbranch(var2dist, D, state, optimizer, max_num_trials,
                            use_dpll, params, unique_var_ids);
                if (state.num_sat_comb == 0) {
                    state.p = (float)total_num_sat_comb / (float)(D.size());
                } else {
                    state.p =
                        (float)total_num_sat_comb / (float)state.num_sat_comb;
                }
                state.num_sat_comb = total_num_sat_comb;
                printf("ttna - %d %f\n", total_num_sat_comb, state.p);
                if (total_num_sat_comb > 0) {
                    is_sat = true;
                }
            } else {
                // solve deterministic path constraints
                if (use_dpll) {
                    smt_dpll_solver(is_sat, state, params, optimizer,
                                    max_num_trials, ignore_memory);
                } else {
                    smt_union_solver(is_sat, state, params, optimizer,
                                     max_num_trials, ignore_memory);
                }
            }

            if (is_sat) {
                maxSAT--;
            } else {
                maxUNSAT--;
            }
            constraints_cache.emplace(constraints_str,
                                      std::make_pair(is_sat, params));
        }

        if (verbose_level >= 1) {
            if ((verbose_level >= 1 && is_unknown_path_constraint &&
                 is_target) ||
                (verbose_level >= 2)) {
                if (!is_sat) {
                    printf("\x1b[31m");
                } else {
                    printf("\x1b[32m");
                }
                printf("pc=%d, IS_SAT - %d\x1b[39m, %s, params = {", pc, is_sat,
                       constraints_str.c_str());
                for (auto &p : params) {
                    // ignore concrete variables
                    if (state.mem.find(p.first) != state.mem.end()) {
                        continue;
                    }
                    // only show symbolic variables
                    if (is_integer(p.second)) {
                        printf("%d: %d, ", p.first, (int)p.second);
                    } else {
                        printf("%d: %f, ", p.first, p.second);
                    }
                }
                printf("}\n");
            }
        }
    }

    if (verbose_level >= 2) {
        printf("---\n");
    }

    if ((prog[pc].instr == InstrType::Done) || (!is_sat)) {
        return Trace(state, {});
    } else if (maxDepth > 0 && maxSAT > 0 && maxUNSAT > 0) {
        Instr instr = prog[pc];
        std::vector<SymState *> newStates;
        symStep(&state, instr, newStates);
        std::vector<Trace> children;
        for (SymState *newState : newStates) {
            Trace child =
                run_pgymbo(prog, var2dist, D, optimizer, *newState, target_pcs,
                           constraints_cache, maxDepth - 1, maxSAT, maxUNSAT,
                           max_num_trials, ignore_memory, use_dpll,
                           verbose_level, return_trace);
            if (return_trace) {
                children.push_back(child);
            }
        }
        return Trace(state, children);
    } else {
        return Trace(state, {});
    }
}

}  // namespace gymbo

