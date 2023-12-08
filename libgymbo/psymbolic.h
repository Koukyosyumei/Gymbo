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

namespace gymbo {

inline int pbranch(std::unordered_map<int, DiscreteDist> &var2dist,
                   SymState &state, GDOptimizer &optimizer, int max_num_trials,
                   bool use_dpll, std::unordered_map<int, float> params,
                   std::unordered_set<int> &unique_var_ids) {
    std::vector<int> pvar_ids;
    for (int i : unique_var_ids) {
        if (var2dist.find(i) != var2dist.end()) {
            pvar_ids.emplace_back(i);
        }
    }

    // int num_pvar_combinations = 1;
    std::vector<int> num_pvar_combinations(pvar_ids.size() + 1, 1);
    for (int i = 0; i < pvar_ids.size(); i++) {
        num_pvar_combinations[i + 1] *= var2dist[pvar_ids[i]].vals.size();
    }
    int total_num_pvar_combinations =
        num_pvar_combinations[num_pvar_combinations.size() - 1];

    bool ignore_memory = false;
    int num_sat = 0;

    for (int i = 0; i < total_num_pvar_combinations; i++) {
        bool is_sat = false;
        for (int j = 0; j < pvar_ids.size(); j++) {
            params[pvar_ids[j]] =
                var2dist[pvar_ids[j]].vals[i % num_pvar_combinations[i + 1]];
        }
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

    if (state.path_constraints.size() != 0 && is_target) {
        std::string constraints_str = "";
        for (int i = 0; i < state.path_constraints.size(); i++) {
            constraints_str +=
                "(" + state.path_constraints[i].toString(true) + ") && ";
        }
        constraints_str += " 1";

        std::unordered_map<int, float> params = {};
        initialize_params(params, state, ignore_memory);

        bool is_sat = false;
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
                    pbranch(var2dist, state, optimizer, max_num_trials,
                            use_dpll, params, unique_var_ids);
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

    if (prog[pc].instr == InstrType::Done) {
        return Trace(state, {});
    } else if (maxDepth > 0 && maxSAT > 0 && maxUNSAT > 0) {
        Instr instr = prog[pc];
        std::vector<SymState *> newStates;
        symStep(&state, instr, newStates);
        std::vector<Trace> children;
        for (SymState *newState : newStates) {
            Trace child =
                run_pgymbo(prog, var2dist, optimizer, *newState, target_pcs,
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

