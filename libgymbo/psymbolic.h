/**
 * @file psymbolic.h
 * @brief Implementation of symbolic execution for randomized programs.
 * @author Hideaki Takahashi
 */

#pragma once
#include "symbolic.h"
#include "type.h"
#include "utils.h"

namespace gymbo {

/**
 * @brief Perform probabilistic branching based on symbolic execution.
 *
 * Given the symbolic state, this function performs probabilistic branching. It
 * computes the symbolic probability of the state being reached and updates the
 * symbolic state accordingly.
 *
 * @param state The symbolic state for the current execution.
 */
inline void pbranch(SymState &state) {
    int n_path_constraints = state.path_constraints.size();
    if (state.has_observed_p_cond) {
        Sym *n_cond = &state.path_constraints[0];
        Sym *d_cond = &state.path_constraints[0];
        for (int i = 1; i < n_path_constraints - 1; i++) {
            n_cond = new Sym(SymType::SAnd, n_cond, &state.path_constraints[i]);
            d_cond = new Sym(SymType::SAnd, d_cond, &state.path_constraints[i]);
        }
        n_cond = new Sym(SymType::SAnd, n_cond,
                         &state.path_constraints[n_path_constraints - 1]);
        state.cond_p = new SymProb(n_cond, d_cond);
        state.p = state.p->pmul(state.cond_p);
    } else {
        Sym *n_cond;
        if (n_path_constraints == 0) {
            n_cond = new Sym(SymType::SCon, FloatToWord(0.0f));
        } else {
            n_cond = &state.path_constraints[0];
        }
        for (int i = 1; i < n_path_constraints; i++) {
            n_cond = new Sym(SymType::SAnd, n_cond, &state.path_constraints[i]);
        }
        Sym *d_cond = new Sym(SymType::SCon, FloatToWord(1.0f));
        state.p = new SymProb(n_cond, d_cond);
        state.cond_p = state.p;
        state.has_observed_p_cond = true;
    }
}

/**
 * @brief Prints verbose information about probabilistic path constraints.
 *
 * This function prints the path constraints, whether they are satisfiable,
 * the probability of reaching the current state, and other relevant
 * information.
 *
 * @param verbose_level The level of verbosity.
 * @param is_unknown_path_constraint Whether the path constraint is unknown.
 * @param is_target Whether the program counter is a target.
 * @param is_sat Whether the path constraint is satisfiable.
 * @param pc The program counter.
 * @param constraints_str String representation of the path constraints.
 * @param state The symbolic state.
 * @param params Dictionary of symbolic variable values.
 */
inline void verbose_pconstraints(int verbose_level,
                                 bool is_unknown_path_constraint,
                                 bool is_target, bool is_sat, int pc,
                                 std::string constraints_str, SymState &state,
                                 const std::unordered_map<int, float> &params) {
    if (verbose_level >= 1) {
        if ((verbose_level >= 1 && is_unknown_path_constraint && is_target) ||
            (verbose_level >= 2)) {
            if (!is_sat) {
                printf("\x1b[31m");
            } else {
                printf("\x1b[32m");
            }
            printf("pc=%d, IS_SAT - %d\x1b[39m, Pr.REACH - %s, %s, params = {",
                   pc, is_sat, state.cond_p->toString().c_str(),
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

/**
 * @brief Solves path constraints and updates the symbolic state.
 *
 * This function checks the satisfiability of the path constraints using an SMT
 * solver or a probabilistic branching algorithm. It updates the symbolic state
 * based on the result and stores the solution in a cache for future use.
 *
 * @param is_sat Whether the path constraints are satisfiable.
 * @param is_target Whether the program counter is a target.
 * @param pc The program counter.
 * @param random_vars The set of random variable IDs.
 * @param optimizer The optimizer used for guided symbolic execution.
 * @param state The symbolic state.
 * @param constraints_cache Cache for storing and reusing path constraints.
 * @param maxSAT Maximum number of satisfiable paths to explore.
 * @param maxUNSAT Maximum number of unsatisfiable paths to explore.
 * @param max_num_trials Maximum number of trials for satisfiability checking.
 * @param ignore_memory Flag to ignore memory updates during symbolic execution.
 * @param use_dpll Flag indicating whether to use the DPLL solver for
 * satisfiability.
 * @param verbose_level Verbosity level for printing debug information.
 */
inline void solve_pconstraints(bool &is_sat, bool is_target, int pc,
                               std::unordered_set<int> &random_vars,
                               GDOptimizer &optimizer, SymState &state,
                               PathConstraintsTable &constraints_cache,
                               int &maxSAT, int &maxUNSAT, int max_num_trials,
                               bool ignore_memory, bool use_dpll,
                               int verbose_level) {
    std::string constraints_str = state.toString(false);

    std::unordered_map<int, float> params = {};
    initialize_params(params, state, ignore_memory);

    bool is_unknown_path_constraint = true;

    if (constraints_cache.find(constraints_str) != constraints_cache.end()) {
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
            if (random_vars.find(i) != random_vars.end()) {
                is_contain_prob_var = true;
                break;
            }
        }

        if (is_contain_prob_var) {
            // call probabilistic branch algorithm
            pbranch(state);
            is_sat = true;
        } else {
            // solve deterministic path constraints
            call_smt_solver(is_sat, state, params, optimizer, max_num_trials,
                            ignore_memory, use_dpll);
            if (is_sat) {
                maxSAT--;
            } else {
                maxUNSAT--;
            }

            if (is_sat) {
                state.p =
                    new SymProb(new Sym(SymType::SCon, FloatToWord(0.0f)),
                                new Sym(SymType::SCon, FloatToWord(0.0f)));
                state.cond_p = state.p;
            }
        }

        constraints_cache.emplace(constraints_str,
                                  std::make_pair(is_sat, params));
    }

    if (verbose_level >= 1) {
        verbose_pconstraints(verbose_level, is_unknown_path_constraint,
                             is_target, is_sat, pc, constraints_str, state,
                             params);
    }
}

/**
 * @brief Updates the table of probabilistic path constraints.
 *
 * This function stores the path constraints and the corresponding probability
 * of reaching the current state in a table for future reference.
 *
 * @param pc The program counter.
 * @param state The symbolic state.
 * @param prob_constraints_table Table for storing probabilistic path
 * constraints.
 */
inline void update_prob_constraints_table(
    int pc, SymState &state, ProbPathConstraintsTable &prob_constraints_table) {
    Sym cc = state.path_constraints[0];
    for (int i = 1; i < state.path_constraints.size(); i++) {
        cc = Sym(SymType::SAnd, cc.copy(), &state.path_constraints[i]);
    }
    if (prob_constraints_table.find(pc) == prob_constraints_table.end()) {
        std::vector<std::tuple<Sym, Mem, SymProb>> tmp = {
            std::make_tuple(cc, state.mem, *state.p)};
        prob_constraints_table.emplace(pc, tmp);
    } else {
        prob_constraints_table[pc].emplace_back(
            std::make_tuple(cc, state.mem, *state.p));
    }
}

/**
 * @brief Run probabilistic symbolic execution on a program.
 *
 * This function performs probabilistic symbolic execution on a given program,
 * considering variable distributions, symbolic states, and path constraints. It
 * explores different execution paths and updates the constraints and
 * probabilities accordingly.
 *
 * @param prog The program to be executed symbolically.
 * @param random_vars The set of random variables'IDs.
 * @param optimizer The optimizer used for guided symbolic execution.
 * @param state The initial symbolic state for execution.
 * @param target_pcs Set of target program counters for analysis.
 * @param constraints_cache Cache for storing and reusing path constraints.
 * @param prob_constraints_table Table for storing probabilistic path
 * constraints.
 * @param maxDepth Maximum exploration depth during symbolic execution.
 * @param maxSAT Maximum number of satisfiable paths to explore.
 * @param maxUNSAT Maximum number of unsatisfiable paths to explore.
 * @param max_num_trials Maximum number of trials for satisfiability checking.
 * @param ignore_memory Flag to ignore memory updates during symbolic execution.
 * @param use_dpll Flag indicating whether to use the DPLL solver for
 * satisfiability.
 * @param verbose_level Verbosity level for printing debug information.
 * @param return_trace Flag to indicate whether to return the symbolic execution
 * trace.
 * @return The symbolic execution trace containing states and child traces.
 */
inline Trace run_pgymbo(Prog &prog, std::unordered_set<int> &random_vars,
                        GDOptimizer &optimizer, SymState &state,
                        std::unordered_set<int> &target_pcs,
                        PathConstraintsTable &constraints_cache,
                        ProbPathConstraintsTable &prob_constraints_table,
                        int maxDepth, int &maxSAT, int &maxUNSAT,
                        int max_num_trials, bool ignore_memory, bool use_dpll,
                        int verbose_level, bool return_trace = false) {
    int pc = state.pc;
    bool is_target = is_target_pc(target_pcs, pc);
    bool is_sat = true;

    verbose_pre(verbose_level, pc, prog, state);
    if (state.path_constraints.size() != 0 && is_target) {
        solve_pconstraints(is_sat, is_target, pc, random_vars, optimizer, state,
                           constraints_cache, maxSAT, maxUNSAT, max_num_trials,
                           ignore_memory, use_dpll, verbose_level);
    }
    verbose_post(verbose_level);

    if (prog[pc].instr == InstrType::Done &&
        state.path_constraints.size() > 0) {
        update_prob_constraints_table(pc, state, prob_constraints_table);
    }

    if ((prog[pc].instr == InstrType::Done) || (!is_sat)) {
        return Trace(state, {});
    } else if (explore_further(maxDepth, maxSAT, maxUNSAT)) {
        Instr instr = prog[pc];
        std::vector<SymState *> newStates;
        symStep(&state, instr, newStates);
        std::vector<Trace> children;
        for (SymState *newState : newStates) {
            Trace child = run_pgymbo(
                prog, random_vars, optimizer, *newState, target_pcs,
                constraints_cache, prob_constraints_table, maxDepth - 1, maxSAT,
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

