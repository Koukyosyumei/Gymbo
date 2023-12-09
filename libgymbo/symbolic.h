/**
 * @file symbolic.h
 * @brief Core implementation of gradient-based symbolic execution
 * @author Hideaki Takahashi
 */

#pragma once
#include "smt.h"

namespace gymbo {
Trace run_gymbo(Prog &prog, GDOptimizer &optimizer, SymState &state,
                std::unordered_set<int> &taregt_pcs,
                PathConstraintsTable &constraints_cache, int maxDepth,
                int &maxSAT, int &maxUNSAT, int max_num_trials,
                bool ignore_memory, bool use_dpll, int verbose_level,
                bool return_trace);
void symStep(SymState *state, Instr &instr, std::vector<SymState *> &);

/**
 * @brief Symbolically Execute a Program with Gradient Descent Optimization.
 *
 * This function conducts symbolic execution of a given program while
 * simultaneously optimizing the path constraints using the provided gradient
 * descent optimizer, `GDOptimizer`.
 *
 * @param prog The program to symbolically execute.
 * @param optimizer The gradient descent optimizer for parameter optimization.
 * @param state The initial symbolic state of the program.
 * @param target_pcs The set of pc where gymbo executes path-constraints
 * solving. If this set is empty or contains -1, gymbo solves all
 * path-constraints.
 * @param constraints_cache A cache for previously found path constraints.
 * @param maxDepth The maximum depth of symbolic exploration.
 * @param maxSAT The maximum number of SAT constraints to collect.
 * @param maxUNSAT The maximum number of UNSAT constraints to collect.
 * @param max_num_trials The maximum number of trials for each gradient descent.
 * @param ignore_memory If set to true, constraints derived from memory will be
 * ignored.
 * @param use_dpll If set to true, use DPLL to decide the initial assignment for
 * each term.
 * @param verbose_level The level of verbosity.
 * @param return_trace If set to true, save the trace at each pc and return them
 * (default false).
 * @return A trace of the symbolic execution.
 */
inline Trace run_gymbo(Prog &prog, GDOptimizer &optimizer, SymState &state,
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
            if (use_dpll) {
                smt_dpll_solver(is_sat, state, params, optimizer,
                                max_num_trials, ignore_memory);
            } else {
                smt_union_solver(is_sat, state, params, optimizer,
                                 max_num_trials, ignore_memory);
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
            Trace child = run_gymbo(prog, optimizer, *newState, target_pcs,
                                    constraints_cache, maxDepth - 1, maxSAT,
                                    maxUNSAT, max_num_trials, ignore_memory,
                                    use_dpll, verbose_level, return_trace);
            if (return_trace) {
                children.push_back(child);
            }
        }
        return Trace(state, children);
    } else {
        return Trace(state, {});
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
}  // namespace gymbo
