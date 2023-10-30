#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "type.h" // Assuming you have a Types.h file with enum Instr defined
#include "utils.h" // Assuming you have a Util.h file with required functions and headers

// Import statements are not directly translatable to C++.
// Include the necessary headers directly.

Trace symRun(int maxDepth, Prog &prog, SymState &state);
void symStep(SymState &state, Instr instr, std::vector<SymState> &);

inline Trace symRun(int maxDepth, Prog &prog, SymState &state) {
  int pc = state.pc;
  // Extract other elements from the state, e.g., mem, stack, cs

  if (prog[pc].instr == InstrType::Done) {
    return Trace(
        state,
        {}); // Construct a Trace with the current state and empty children
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
    return Trace(
        state,
        {}); // Construct a Trace with the current state and empty children
  }
}

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
    Sym *l = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *r = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    new_state.pc++;
    new_state.symbolic_stack.push(Sym(SymType::SAdd, l, r));
    result.emplace_back(new_state);
    break;
  }
  case InstrType::And: {
    Sym *l = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *r = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    new_state.pc++;
    new_state.symbolic_stack.push(Sym(SymType::SAnd, l, r));
    result.emplace_back(new_state);
    break;
  }
  case InstrType::Or: {
    Sym *l = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *r = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    new_state.pc++;
    new_state.symbolic_stack.push(Sym(SymType::SOr, l, r));
    result.emplace_back(new_state);
    break;
  }
  case InstrType::Lt: {
    Sym *l = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *r = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    new_state.pc++;
    new_state.symbolic_stack.push(Sym(SymType::SLt, l, r));
    result.emplace_back(new_state);
    break;
  }
  case InstrType::Eq: {
    Sym *l = new_state.symbolic_stack.back();
    new_state.symbolic_stack.pop();
    Sym *r = new_state.symbolic_stack.back();
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
    new_state.pc++;
    new_state.symbolic_stack.push(*w);
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
      true_state.pc = wordToInt(addr->word);
      true_state.path_constraints.emplace_back(*cond);
      false_state.pc++;
      false_state.path_constraints.emplace_back(Sym(SymType::SNot, cond));
    }
    break;
  }
  default:
    throw std::runtime_error("Unsupported instruction");
  }
}
