#include "gd.h"
#include "type.h"
#include "utils.h"
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace gymbo {
Trace run_gymbo(Prog &prog, GDOptimizer &optimizer, SymState &state,
                PathConstraintsTable &, int maxDepth, bool ignore_memory,
                int verbose_level);
void symStep(SymState &state, Instr instr, std::vector<SymState> &);

/**
 * Performs symbolic execution of a program.
 *
 * @param prog The program to be symbolically executed.
 * @param state The initial state of the program.
 * @constrains_cache The cache for already found path constraints
 * @param maxDepth The maximum depth of the symbolic exploration tree.
 * @return A trace of the symbolic execution.
 */
inline Trace run_gymbo(Prog &prog, GDOptimizer &optimizer, SymState &state,
                       PathConstraintsTable &constraints_cache,
                       int maxDepth = 64, bool ignore_memory = false,
                       int verbose_level = 1) {
  int pc = state.pc;
  if (verbose_level >= 1) {
    printf("pc: %d, ", pc);
    prog[pc].print();
    state.print();
  }

  if (state.path_constraints.size() != 0) {
    std::string constraints_str = "";
    for (int i = 0; i < state.path_constraints.size(); i++) {
      constraints_str += state.path_constraints[i].toString() + " and ";
    }
    constraints_str += " 1";

    std::unordered_map<int, int> params = {};
    if (!ignore_memory) {
      for (auto &p : state.mem) {
        params.emplace(std::make_pair(p.first, p.second));
      }
    }

    bool is_sat;

    if (constraints_cache.find(constraints_str) != constraints_cache.end()) {
      is_sat = constraints_cache[constraints_str].first;
      params = constraints_cache[constraints_str].second;
    } else {
      is_sat = optimizer.solve(state.path_constraints, params);
      constraints_cache.emplace(constraints_str,
                                std::make_pair(is_sat, params));
    }

    if (verbose_level >= 1) {
      if (!is_sat) {
        printf("\x1b[31m");
      } else {
        printf("\x1b[32m");
      }
      printf("IS_SAT - %d\x1b[39m, params = {", is_sat);
      for (auto &p : params) {
        printf("%d: %d, ", p.first, p.second);
      }
      printf("}\n");
    }
  }

  if (verbose_level >= 1) {
    printf("---\n");
  }

  if (prog[pc].instr == InstrType::Done) {
    return Trace(state, {});
  } else if (maxDepth > 0) {
    Instr instr = prog[pc];
    std::vector<SymState> newStates;
    symStep(state, instr, newStates);
    std::vector<Trace> children;
    for (SymState newState : newStates) {
      Trace child = run_gymbo(prog, optimizer, newState, constraints_cache,
                              maxDepth - 1, ignore_memory, verbose_level);
      children.push_back(child);
    }
    return Trace(state, children);
  } else {
    return Trace(state, {});
  }
}

/**
 * Symbolically executes a single instruction of a program.
 *
 * @param state The state of the program before the instruction is executed.
 * @param instr The instruction to be executed.
 * @param result A list of new states, each of which represents a possible
 * outcome of executing the instruction.
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
      if (new_state.mem.find(wordToInt(addr->var_idx)) == new_state.mem.end()) {
        new_state.mem.emplace(wordToInt(addr->var_idx), w->word);
      } else {
        new_state.mem.at(wordToInt(addr->var_idx)) = w->word;
      }
    } else if (w->symtype == SymType::SAny) {
      if (new_state.mem.find(wordToInt(w->var_idx)) != new_state.mem.end()) {
        if (new_state.mem.find(wordToInt(addr->var_idx)) ==
            new_state.mem.end()) {
          new_state.mem.emplace(wordToInt(addr->var_idx),
                                new_state.mem[wordToInt(w->var_idx)]);
        } else {
          new_state.mem.at(wordToInt(addr->var_idx)) =
              new_state.mem[wordToInt(w->var_idx)];
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
    new_state.symbolic_stack.push(Sym(SymType::SAny, addr->word));
    new_state.pc++;
    result.emplace_back(new_state);
    break;
  }
  case InstrType::Read: {
    new_state.symbolic_stack.push(Sym(SymType::SAny, new_state.var_cnt));
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
    Sym *cond = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *addr = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    if (addr->symtype == SymType::SCon) {
      SymState true_state = new_state;
      SymState false_state = new_state;
      true_state.pc += wordToInt(addr->word - 2);
      true_state.path_constraints.emplace_back(*cond);
      false_state.pc++;
      false_state.path_constraints.emplace_back(Sym(SymType::SNot, cond));
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
    throw std::runtime_error("Unsupported instruction");
  }
}
} // namespace gymbo
