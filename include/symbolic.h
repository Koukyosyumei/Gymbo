#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "type.h"
#include "utils.h"

Trace symRun(int maxDepth, Prog &prog, SymState &state);
void symStep(SymState &state, Instr instr, std::vector<SymState> &);

/**
 * Performs symbolic execution of a program.
 *
 * @param maxDepth The maximum depth of the symbolic exploration tree.
 * @param prog The program to be symbolically executed.
 * @param state The initial state of the program.
 * @return A trace of the symbolic execution.
 */
inline Trace symRun(int maxDepth, Prog &prog, SymState &state) {
  int pc = state.pc;
  printf("pc: %d, ", pc);
  prog[pc].print();
  state.print();
  printf("---\n");

  if (prog[pc].instr == InstrType::Done) {
    return Trace(state, {});
  } else if (maxDepth > 0) {
    Instr instr = prog[pc];
    std::vector<SymState> newStates;
    symStep(state, instr, newStates);
    std::vector<Trace> children;
    for (SymState newState : newStates) {
      Trace child = symRun(maxDepth - 1, prog, newState);
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
    new_state.mem.emplace(wordToInt(addr->var_idx), w->word);
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
