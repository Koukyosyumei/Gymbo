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

/**
 * @brief Compute the Cartesian product of a vector of vectors of integers.
 *
 * This function takes a vector of vectors of integers and computes their
 * Cartesian product. The result is a vector of vectors, where each inner vector
 * represents a combination of elements from the input vectors.
 *
 * @param vectors A vector of vectors of integers for which the Cartesian
 * product is computed.
 * @return The Cartesian product as a vector of vectors of integers.
 */
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

/**
 * @brief Perform probabilistic branching based on symbolic execution.
 *
 * Given a set of variable distributions, symbolic state, and path constraints,
 * this function performs probabilistic branching. It computes the number of
 * satisfying combinations for probabilistic variables and updates the symbolic
 * state accordingly.
 *
 * @param var2dist A map of variable IDs to their discrete distributions.
 * @param D A vector of vectors representing the all possible combinations of
 * probabilistic variables.
 * @param state The symbolic state for the current execution.
 * @param optimizer The optimizer used for guided symbolic execution.
 * @param max_num_trials The maximum number of trials for satisfiability
 * checking.
 * @param use_dpll Flag indicating whether to use the DPLL solver for
 * satisfiability.
 * @param params Initial parameters for symbolic execution.
 * @param unique_var_ids Set of unique variable IDs for probabilistic branching.
 * @return The number of satisfying combinations after probabilistic branching.
 */
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

/**
 * @brief Run probabilistic symbolic execution on a program.
 *
 * This function performs probabilistic symbolic execution on a given program, considering
 * variable distributions, symbolic states, and path constraints. It explores different
 * execution paths and updates the constraints and probabilities accordingly.
 *
 * @param prog The program to be executed symbolically.
 * @param var2dist Map of variable IDs to their discrete distributions.
 * @param D A vector of vectors representing the all possible combinations of
 * probabilistic variables.
 * @param optimizer The optimizer used for guided symbolic execution.
 * @param state The initial symbolic state for execution.
 * @param target_pcs Set of target program counters for analysis.
 * @param constraints_cache Cache for storing and reusing path constraints.
 * @param prob_constraints_table Table for storing probabilistic path constraints.
 * @param maxDepth Maximum exploration depth during symbolic execution.
 * @param maxSAT Maximum number of satisfiable paths to explore.
 * @param maxUNSAT Maximum number of unsatisfiable paths to explore.
 * @param max_num_trials Maximum number of trials for satisfiability checking.
 * @param ignore_memory Flag to ignore memory updates during symbolic execution.
 * @param use_dpll Flag indicating whether to use the DPLL solver for satisfiability.
 * @param verbose_level Verbosity level for printing debug information.
 * @param return_trace Flag to indicate whether to return the symbolic execution trace.
 * @return The symbolic execution trace containing states and child traces.
 */
inline Trace run_pgymbo(Prog &prog,
                        std::unordered_map<int, DiscreteDist> &var2dist,
                        std::vector<std::vector<int>> &D,
                        GDOptimizer &optimizer, SymState &state,
                        std::unordered_set<int> &target_pcs,
                        PathConstraintsTable &constraints_cache,
                        ProbPathConstraintsTable &prob_constrains_table,
                        int maxDepth, int &maxSAT, int &maxUNSAT,
                        int max_num_trials, bool ignore_memory, bool use_dpll,
                        int verbose_level, bool return_trace = false) {
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
        std::string constraints_str = state.toString(false);

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
                    state.p *= (float)total_num_sat_comb / (float)(D.size());
                } else {
                    state.p *=
                        (float)total_num_sat_comb / (float)state.num_sat_comb;
                }

                state.num_sat_comb = total_num_sat_comb;

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

                state.p *= (float)is_sat;
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
                printf(
                    "pc=%d, IS_SAT - %d\x1b[39m, Pr.REACH - %f, %s, params = {",
                    pc, is_sat, state.p, constraints_str.c_str());
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
        Sym cc = Sym(SymType::SCon, FloatToWord(1.0f));
        for (Sym &s : state.path_constraints) {
            cc = Sym(SymType::SAnd, cc.copy(), &s);
        }
        if (prob_constrains_table.find(pc) == prob_constrains_table.end()) {
            std::vector<std::tuple<Sym, Mem, float>> tmp = {
                std::make_tuple(cc, state.mem, state.p)};
            prob_constrains_table.emplace(pc, tmp);
        } else {
            prob_constrains_table[pc].emplace_back(
                std::make_tuple(cc, state.mem, state.p));
        }
    }

    if ((prog[pc].instr == InstrType::Done) || (!is_sat)) {
        return Trace(state, {});
    } else if (maxDepth > 0 && maxSAT > 0 && maxUNSAT > 0) {
        Instr instr = prog[pc];
        std::vector<SymState *> newStates;
        symStep(&state, instr, newStates);
        std::vector<Trace> children;
        for (SymState *newState : newStates) {
            Trace child = run_pgymbo(
                prog, var2dist, D, optimizer, *newState, target_pcs,
                constraints_cache, prob_constrains_table, maxDepth - 1, maxSAT,
                maxUNSAT, max_num_trials, ignore_memory, use_dpll,
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

