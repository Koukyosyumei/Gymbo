/**
 * @file symbolic.h
 * @brief Core implementation of gradient-based symbolic execution
 * @author Hideaki Takahashi
 */

#pragma once
#include "gd.h"
#include "sat.h"

namespace gymbo {
Trace run_gymbo(Prog &prog, Optimizer &optimizer, SymState &state,
                std::unordered_set<int> &taregt_pcs,
                PathConstraintsTable &constraints_cache, int maxDepth,
                int &maxSAT, int &maxUNSAT, int max_num_trials,
                bool ignore_memory, bool use_dpll, int verbose_level);
void symStep(SymState &state, Instr instr, std::vector<SymState> &);

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
void initialize_params(std::unordered_map<int, float> &params, SymState &state,
                       bool ignore_memory) {
    params = {};
    if (!ignore_memory) {
        for (auto &p : state.mem) {
            params.emplace(std::make_pair(p.first, wordToFloat(p.second)));
        }
    }
}

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
 * @return A trace of the symbolic execution.
 */
inline Trace run_gymbo(Prog &prog, Optimizer &optimizer, SymState &state,
                       std::unordered_set<int> &target_pcs,
                       PathConstraintsTable &constraints_cache, int maxDepth,
                       int &maxSAT, int &maxUNSAT, int max_num_trials,
                       bool ignore_memory, bool use_dpll, int verbose_level) {
    int pc = state.pc;
    if (verbose_level >= 2) {
        printf("pc: %d, ", pc);
        prog[pc].print();
        state.print();
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
            std::unordered_map<std::string, gymbo::Sym *> unique_terms_map;
            std::unordered_map<std::string, bool> assignments_map;
            std::shared_ptr<gymbosat::Expr> path_constraints_expr =
                gymbosat::pathconstraints2expr(state.path_constraints,
                                               unique_terms_map);

            if (use_dpll) {
                while (
                    satisfiableDPLL(path_constraints_expr, assignments_map)) {
                    std::vector<Sym> new_constraints;
                    for (auto &ass : assignments_map) {
                        if (ass.second) {
                            new_constraints.emplace_back(
                                *unique_terms_map[ass.first]);
                        } else {
                            new_constraints.emplace_back(Sym(
                                SymType::SNot, unique_terms_map[ass.first]));
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
                                    std::make_shared<gymbosat::Var>(
                                        ass.first)));
                        } else {
                            learnt_constraints = std::make_shared<gymbosat::Or>(
                                learnt_constraints,
                                std::make_shared<gymbosat::Var>(ass.first));
                        }
                    }

                    path_constraints_expr = std::make_shared<gymbosat::And>(
                        path_constraints_expr, learnt_constraints);
                }
            } else {
                for (int j = 0; j < max_num_trials; j++) {
                    is_sat = optimizer.solve(state.path_constraints, params);
                    if (is_sat) {
                        break;
                    }
                    optimizer.seed += 1;
                    initialize_params(params, state, ignore_memory);
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
        std::vector<SymState> newStates;
        symStep(state, instr, newStates);
        std::vector<Trace> children;
        for (SymState newState : newStates) {
            Trace child = run_gymbo(prog, optimizer, newState, target_pcs,
                                    constraints_cache, maxDepth - 1, maxSAT,
                                    maxUNSAT, max_num_trials, ignore_memory,
                                    use_dpll, verbose_level);
            children.push_back(child);
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
inline void symStep(SymState &state, Instr instr,
                    std::vector<SymState> &result) {
    SymState new_state = state;

    switch (instr.instr) {
        case InstrType::Not: {
            Sym *w = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SNot, w));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Add: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SAdd, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Sub: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SSub, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Mul: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SMul, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::And: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SAnd, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Or: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SOr, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Lt: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SLt, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Le: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SLe, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Eq: {
            Sym *r = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *l = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(Sym(SymType::SEq, l, r));
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Swap: {
            Sym *x = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *y = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(*x);
            new_state.symbolic_stack.push(*y);
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Store: {
            Sym *addr = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            Sym *w = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            if (w->symtype == SymType::SCon) {
                if (new_state.mem.find(wordToInt(addr->var_idx)) ==
                    new_state.mem.end()) {
                    new_state.mem.emplace(wordToInt(addr->var_idx), w->word);
                } else {
                    new_state.mem.at(wordToInt(addr->var_idx)) = w->word;
                }
            } else if ((w->symtype == SymType::SAny) &&
                       (new_state.mem.find(wordToInt(w->var_idx)) !=
                        new_state.mem.end())) {
                if (new_state.mem.find(wordToInt(addr->var_idx)) ==
                    new_state.mem.end()) {
                    new_state.mem.emplace(wordToInt(addr->var_idx),
                                          new_state.mem[wordToInt(w->var_idx)]);
                } else {
                    new_state.mem.at(wordToInt(addr->var_idx)) =
                        new_state.mem[wordToInt(w->var_idx)];
                }
            } else {
                if (new_state.smem.find(wordToInt(addr->var_idx)) ==
                    new_state.smem.end()) {
                    if (new_state.smem.find(wordToInt(w->var_idx)) ==
                        new_state.smem.end()) {
                        new_state.smem.emplace(wordToInt(addr->var_idx), *w);
                    } else {
                        new_state.smem.emplace(
                            wordToInt(addr->var_idx),
                            new_state.smem[wordToInt(w->var_idx)]);
                    }
                } else {
                    if (new_state.smem.find(wordToInt(w->var_idx)) ==
                        new_state.smem.end()) {
                        new_state.smem.at(wordToInt(addr->var_idx)) = *w;
                    } else {
                        new_state.smem.at(wordToInt(addr->var_idx)) =
                            new_state.smem[wordToInt(w->var_idx)];
                    }
                }
            }
            new_state.pc++;
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Load: {
            Sym *addr = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            if (new_state.smem.find(wordToInt(addr->word)) !=
                new_state.smem.end()) {
                new_state.symbolic_stack.push(
                    new_state.smem[wordToInt(addr->word)]);
            } else {
                new_state.symbolic_stack.push(Sym(SymType::SAny, addr->word));
            }
            new_state.pc++;
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Read: {
            new_state.symbolic_stack.push(
                Sym(SymType::SAny, new_state.var_cnt));
            new_state.pc++;
            new_state.var_cnt++;
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Push: {
            new_state.symbolic_stack.push(Sym(SymType::SCon, instr.word));
            new_state.pc++;
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Dup: {
            Sym *w = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc++;
            new_state.symbolic_stack.push(*w);
            new_state.symbolic_stack.push(*w);
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Pop: {
            new_state.symbolic_stack.pop();
            new_state.pc++;
            result.emplace_back(new_state);
            break;
        }
        case InstrType::JmpIf: {
            Sym *cond =
                new_state.symbolic_stack.back()->psimplify(new_state.mem);
            new_state.symbolic_stack.pop();
            Sym *addr = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            if (addr->symtype == SymType::SCon) {
                SymState true_state = new_state;
                SymState false_state = new_state;
                true_state.pc += wordToInt(addr->word - 2);
                true_state.path_constraints.emplace_back(*cond);
                false_state.pc++;
                false_state.path_constraints.emplace_back(
                    Sym(SymType::SNot, cond));
                result.emplace_back(true_state);
                result.emplace_back(false_state);
            }
            break;
        }
        case InstrType::Jmp: {
            Sym *addr = new_state.symbolic_stack.back();
            new_state.symbolic_stack.pop();
            new_state.pc += wordToInt(addr->word);
            result.emplace_back(new_state);
            break;
        }
        case InstrType::Nop: {
            new_state.pc++;
            result.emplace_back(new_state);
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
