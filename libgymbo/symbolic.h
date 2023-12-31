/**
 * @file symbolic.h
 * @brief Core implementation of gradient-based symbolic execution
 * @author Hideaki Takahashi
 */

#pragma once
#include "smt.h"

namespace gymbo {

/**
 * @brief Checks if the given program counter (pc) is a target program counter.
 *
 * This function checks whether the provided program counter (pc) is part of the
 * set of target program counters.
 *
 * @param target_pcs The set of target program counters.
 * @param pc The program counter to be checked.
 * @return True if the pc is a target program counter, false otherwise.
 */
inline bool is_target_pc(const std::unordered_set<int> &target_pcs, int pc) {
    return ((target_pcs.size() == 0) ||
            (target_pcs.find(-1) != target_pcs.end()) ||
            (target_pcs.find(pc) != target_pcs.end()));
}

/**
 * @brief Checks if further exploration is allowed based on maximum depth and
 * satisfiability conditions.
 *
 * This function determines whether further exploration is allowed based on the
 * specified maximum depth and satisfiability conditions.
 *
 * @param maxDepth The maximum depth for exploration.
 * @param maxSAT The maximum satisfiability limit.
 * @param maxUNSAT The maximum unsatisfiability limit.
 * @return True if further exploration is allowed, false otherwise.
 */
inline bool explore_further(int maxDepth, int maxSAT, int maxUNSAT) {
    return maxDepth > 0 && maxSAT > 0 && maxUNSAT > 0;
}

/**
 * @brief Calls the SMT solver based on the specified options.
 *
 * This function calls the SMT solver, either DPLL solver or union solver, based
 * on the specified options.
 *
 * @param is_sat Reference to a boolean indicating satisfiability.
 * @param state Reference to the symbolic state.
 * @param params Reference to a map containing parameters.
 * @param optimizer Reference to the optimizer.
 * @param max_num_trials Maximum number of solver trials.
 * @param ignore_memory Flag indicating whether to ignore memory.
 * @param use_dpll Flag indicating whether to use the DPLL solver.
 */
inline void call_smt_solver(bool &is_sat, SymState &state,
                            std::unordered_map<int, float> &params,
                            GDOptimizer &optimizer, int max_num_trials,
                            bool ignore_memory, bool use_dpll) {
    if (use_dpll) {
        smt_dpll_solver(is_sat, state, params, optimizer, max_num_trials,
                        ignore_memory);
    } else {
        smt_union_solver(is_sat, state, params, optimizer, max_num_trials,
                         ignore_memory);
    }
}

/**
 * @brief Prints a verbose for conflicts solving if conditions are met.
 *
 * This function prints a verbose for conflicts solving if the specified
 * conditions are met.
 *
 * @param verbose_level The verbosity level.
 * @param is_unknown_path_constraint Flag indicating whether the path constraint
 * is unknown.
 * @param is_target Flag indicating whether the program counter is a target.
 * @param is_sat Flag indicating satisfiability.
 * @param pc The program counter.
 * @param constraints_str String representation of path constraints.
 * @param state Reference to the symbolic state.
 * @param params Reference to the map containing parameters.
 */
inline void verbose_constraints(int verbose_level,
                                bool is_unknown_path_constraint, bool is_target,
                                bool is_sat, int pc,
                                std::string constraints_str,
                                const SymState &state,
                                const std::unordered_map<int, float> &params) {
    if ((verbose_level >= 1 && is_unknown_path_constraint && is_target) ||
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

/**
 * @brief Prints a verbose representation before solving constraints.
 *
 * This function prints a verbose representation before solving constraints.
 *
 * @param verbose_level The verbosity level.
 * @param pc The program counter.
 * @param prog Reference to the program.
 * @param state Reference to the symbolic state.
 */
inline void verbose_pre(int verbose_level, int pc, Prog &prog,
                        SymState &state) {
    if (verbose_level > -1) {
        printf("pc: %d, ", pc);
        prog[pc].print();
        if (verbose_level >= 2) {
            state.print();
        }
    }
}

/**
 * @brief Prints a verbose representation after solving constraints.
 *
 * This function prints a verbose representation after solving constraints.
 *
 * @param verbose_level The verbosity level.
 */
inline void verbose_post(int verbose_level) {
    if (verbose_level >= 2) {
        printf("---\n");
    }
}

/**
 * @brief Symbolically Execute a Single Instruction of a Program.
 *
 * This function symbolically executes a single instruction of a program and
 * generates new symbolic states, representing possible outcomes of the
 * instruction's execution.
 *
 * @param state The state of the program before the instruction is executed.
 * @param instr The instruction to be executed.
 * @param result A list of new symbolic states, each representing a possible
 * outcome.
 */
inline void symStep(SymState *state, Instr &instr,
                    std::vector<SymState *> &result) {
    // SymState state = state;

    switch (instr.instr) {
        case InstrType::Not: {
            Sym *w = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SNot, w));
            result.emplace_back(state);
            break;
        }
        case InstrType::Add: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SAdd, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Sub: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SSub, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Mul: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SMul, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::And: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SAnd, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Or: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SOr, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Lt: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SLt, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Le: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SLe, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Eq: {
            Sym *r = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *l = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(Sym(SymType::SEq, l, r));
            result.emplace_back(state);
            break;
        }
        case InstrType::Swap: {
            Sym *x = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *y = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(*x);
            state->symbolic_stack.push(*y);
            result.emplace_back(state);
            break;
        }
        case InstrType::Store: {
            Sym *addr = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            Sym *w = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            if (w->symtype == SymType::SCon) {
                if (state->mem.find(wordToInt(addr->var_idx)) ==
                    state->mem.end()) {
                    state->mem.emplace(wordToInt(addr->var_idx), w->word);
                } else {
                    state->mem.at(wordToInt(addr->var_idx)) = w->word;
                }
            } else if ((w->symtype == SymType::SAny) &&
                       (state->mem.find(wordToInt(w->var_idx)) !=
                        state->mem.end())) {
                if (state->mem.find(wordToInt(addr->var_idx)) ==
                    state->mem.end()) {
                    state->mem.emplace(wordToInt(addr->var_idx),
                                       state->mem[wordToInt(w->var_idx)]);
                } else {
                    state->mem.at(wordToInt(addr->var_idx)) =
                        state->mem[wordToInt(w->var_idx)];
                }
            } else {
                if (state->smem.find(wordToInt(addr->var_idx)) ==
                    state->smem.end()) {
                    if (state->smem.find(wordToInt(w->var_idx)) ==
                        state->smem.end()) {
                        state->smem.emplace(wordToInt(addr->var_idx), *w);
                    } else {
                        state->smem.emplace(wordToInt(addr->var_idx),
                                            state->smem[wordToInt(w->var_idx)]);
                    }
                } else {
                    if (state->smem.find(wordToInt(w->var_idx)) ==
                        state->smem.end()) {
                        state->smem.at(wordToInt(addr->var_idx)) = *w;
                    } else {
                        state->smem.at(wordToInt(addr->var_idx)) =
                            state->smem[wordToInt(w->var_idx)];
                    }
                }
            }
            state->pc++;
            result.emplace_back(state);
            break;
        }
        case InstrType::Load: {
            Sym *addr = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            if (state->smem.find(wordToInt(addr->word)) != state->smem.end()) {
                state->symbolic_stack.push(state->smem[wordToInt(addr->word)]);
            } else {
                state->symbolic_stack.push(Sym(SymType::SAny, addr->word));
            }
            state->pc++;
            result.emplace_back(state);
            break;
        }
        case InstrType::Read: {
            state->symbolic_stack.push(Sym(SymType::SAny, state->var_cnt));
            state->pc++;
            state->var_cnt++;
            result.emplace_back(state);
            break;
        }
        case InstrType::Push: {
            state->symbolic_stack.push(Sym(SymType::SCon, instr.word));
            state->pc++;
            result.emplace_back(state);
            break;
        }
        case InstrType::Dup: {
            Sym *w = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc++;
            state->symbolic_stack.push(*w);
            state->symbolic_stack.push(*w);
            result.emplace_back(state);
            break;
        }
        case InstrType::Pop: {
            state->symbolic_stack.pop();
            state->pc++;
            result.emplace_back(state);
            break;
        }
        case InstrType::JmpIf: {
            Sym *cond = state->symbolic_stack.back()->psimplify(state->mem);
            state->symbolic_stack.pop();
            Sym *addr = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            if (addr->symtype == SymType::SCon) {
                SymState *true_state = state->copy();
                SymState *false_state = state->copy();
                true_state->pc += wordToInt(addr->word - 2);
                true_state->path_constraints.emplace_back(*cond);
                false_state->pc++;
                false_state->path_constraints.emplace_back(
                    Sym(SymType::SNot, cond));
                result.emplace_back(true_state);
                result.emplace_back(false_state);
            }
            break;
        }
        case InstrType::Jmp: {
            Sym *addr = state->symbolic_stack.back();
            state->symbolic_stack.pop();
            state->pc += wordToInt(addr->word);
            result.emplace_back(state);
            break;
        }
        case InstrType::Nop: {
            state->pc++;
            result.emplace_back(state);
            break;
        }
        case InstrType::Done: {
            break;
        }
        default:
            fprintf(stderr, "Detect unsupported instruction\n");
    }
}

/**
 * @struct BaseExecutor
 * @brief Represents the base class for symbolic execution engine.
 */
struct BaseExecutor {
    GDOptimizer optimizer;  ///< The gradient descent optimizer for parameter
                            ///< optimization.
    int maxSAT;    ///< The maximum number of SAT constraints to collect.
    int maxUNSAT;  ///< The maximum number of UNSAT constraints to collect.
    int max_num_trials;  ///< The maximum number of trials for each gradient
                         ///< descent.
    int verbose_level;   ///< The level of verbosity.
    bool ignore_memory;  ///< If set to true, constraints derived from memory
                         ///< will be ignored.
    bool use_dpll;       ///< If set to true, use DPLL to decide the initial
                         ///< assignment for each term.
    bool return_trace;   ///< If set to true, save the trace at each pc and
                         ///< return them.

    /**
     * @brief Constructor for BaseExecutor.
     *
     * @param optimizer The gradient descent optimizer for parameter
     * optimization.
     * @param maxSAT The maximum number of SAT constraints to collect.
     * @param maxUNSAT The maximum number of UNSAT constraints to collect.
     * @param max_num_trials The maximum number of trials for each gradient
     * descent.
     * @param ignore_memory If set to true, constraints derived from memory will
     * be ignored.
     * @param use_dpll If set to true, use DPLL to decide the initial assignment
     * for each term.
     * @param verbose_level The level of verbosity.
     * @param return_trace If set to true, save the trace at each pc and return
     * them (default false).
     */
    BaseExecutor(GDOptimizer optimizer, int maxSAT = 256, int maxUNSAT = 256,
                 int max_num_trials = 10, bool ignore_memory = false,
                 bool use_dpll = false, int verbose_level = 0,
                 bool return_trace = false)
        : optimizer(optimizer),
          maxSAT(maxSAT),
          maxUNSAT(maxUNSAT),
          max_num_trials(max_num_trials),
          ignore_memory(ignore_memory),
          use_dpll(use_dpll),
          verbose_level(verbose_level),
          return_trace(return_trace){};

    virtual bool solve(bool is_target, int pc, SymState &state) = 0;
    virtual Trace run(Prog &prog, std::unordered_set<int> &target_pcs,
                      SymState &state, int maxDepth) = 0;
};

/**
 * @struct SExecutor
 * @brief Represents a derived class for symbolic execution engine for
 * deterministic programs.
 */
struct SExecutor : public BaseExecutor {
    PathConstraintsTable
        constraints_cache;  ///< Cache for storing and reusing path constraints.

    using BaseExecutor::BaseExecutor;

    /**
     * @brief Solves path constraints and updates the cache.
     *
     * This function solves path constraints, updates the cache, and prints
     * verbose information if conditions are met.
     *
     * @param is_target Flag indicating whether the program counter is a target.
     * @param pc The program counter.
     * @param state Reference to the symbolic state.
     * @return The flag indicating satisfifiability.
     */
    bool solve(bool is_target, int pc, SymState &state) {
        bool is_sat = true;
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
            call_smt_solver(is_sat, state, params, optimizer, max_num_trials,
                            ignore_memory, use_dpll);
            if (is_sat) {
                maxSAT--;
            } else {
                maxUNSAT--;
            }
            constraints_cache.emplace(constraints_str,
                                      std::make_pair(is_sat, params));
        }

        if (verbose_level >= 1) {
            verbose_constraints(verbose_level, is_unknown_path_constraint,
                                is_target, is_sat, pc, constraints_str, state,
                                params);
        }

        return is_sat;
    }

    /**
     * @brief Symbolically Execute a Program with Gradient Descent Optimization.
     *
     * This function conducts symbolic execution of a given program while
     * simultaneously optimizing the path constraints using the provided
     * gradient descent optimizer, `GDOptimizer`.
     *
     * @param prog The program to symbolically execute.
     * @param target_pcs The set of pc where gymbo executes path-constraints
     * solving. If this set is empty or contains -1, gymbo solves all
     * path-constraints.
     * @param state The initial symbolic state of the program.
     * @param maxDepth The maximum depth of symbolic exploration.
     * @return A trace of the symbolic execution.
     */
    Trace run(Prog &prog, std::unordered_set<int> &target_pcs, SymState &state,
              int maxDepth = 256) {
        int pc = state.pc;
        bool is_target = is_target_pc(target_pcs, pc);
        bool is_sat = true;

        verbose_pre(verbose_level, pc, prog, state);

        if (state.path_constraints.size() != 0 && is_target) {
            is_sat = solve(is_target, pc, state);
        }
        verbose_post(verbose_level);

        if ((prog[pc].instr == InstrType::Done) || (!is_sat)) {
            return Trace(state, {});
        } else if (explore_further(maxDepth, maxSAT, maxUNSAT)) {
            Instr instr = prog[pc];
            std::vector<SymState *> newStates;
            symStep(&state, instr, newStates);
            std::vector<Trace> children;
            for (SymState *newState : newStates) {
                Trace child = run(prog, target_pcs, *newState, maxDepth - 1);
                if (return_trace) {
                    children.push_back(child);
                }
            }
            return Trace(state, children);
        } else {
            return Trace(state, {});
        }
    }
};

}  // namespace gymbo
