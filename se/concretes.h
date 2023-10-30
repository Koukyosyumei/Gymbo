#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "type.h" // Assuming you have a Types.h file with enum InstrType defined
#include "utils.h" // Assuming you have a Util.h file with required functions and headers

using namespace std;

// Import statements are not directly translatable to C++.
// Include the necessary headers directly.

vector<Word32> run(bool trace, Prog &prg, State &st);
State step(State st, Instr instr);

inline vector<Word32> run(bool trace, Prog &prg, State &st) {
  int pc = st.pc;
  Mem &mem = st.mem;
  vector<Word32> &stack = st.stack;

  if (prg[pc].instr == InstrType::Done) {
    return stack;
  } else {
    Instr instr = prg[pc];
    if (trace) {
      cout << "Trace: " << pc << " " << static_cast<int>(instr.instr) << endl;
    }
    State newState = step(st, instr);
    return run(trace, prg, newState);
  }
}

inline State step(State st, InstrType instr) {
  int pc = st.pc;
  Mem &mem = st.mem;
  vector<Word32> &stack = st.stack;

  switch (instr) {
  case InstrType::Add: {
    if (stack.size() < 2) {
      throw runtime_error("`Add` expects two arguments.");
    }
    Word32 r = stack.back();
    stack.pop_back();
    Word32 l = stack.back();
    stack.pop_back();
    stack.push_back(l + r);
    return State(pc + 1, mem, stack);
  }
  case InstrType::JmpIf: {
    if (stack.size() < 2) {
      throw runtime_error("`JmpIf` expects 2 arguments.");
    }
    Word32 cond = stack.back();
    stack.pop_back();
    Word32 addr = stack.back();
    stack.pop_back();
    if (cond == 0) {
      return State(pc + 1, mem, stack);
    } else {
      return State(wordToInt(addr), mem, stack);
    }
  }
  case InstrType::And: {
    if (stack.size() < 2) {
      throw runtime_error("And expects two arguments.");
    }
    Word32 r = stack.back();
    stack.pop_back();
    Word32 l = stack.back();
    stack.pop_back();
    bool lValue = wordToBool(l);
    bool rValue = wordToBool(r);
    bool result = lValue && rValue;
    stack.push_back(boolToWord(result));
    return State(pc + 1, mem, stack);
  }
  case InstrType::Or: {
    if (stack.size() < 2) {
      throw runtime_error("Or expects two arguments.");
    }
    Word32 r = stack.back();
    stack.pop_back();
    Word32 l = stack.back();
    stack.pop_back();
    bool lValue = wordToBool(l);
    bool rValue = wordToBool(r);
    bool result = lValue || rValue;
    stack.push_back(boolToWord(result));
    return State(pc + 1, mem, stack);
  }
  case InstrType::Not: {
    if (stack.empty()) {
      throw runtime_error("Not expects one argument.");
    }
    Word32 a = stack.back();
    stack.pop_back();
    bool aBool = wordToBool(a);
    bool result = !aBool;
    stack.push_back(boolToWord(result));
    return State(pc + 1, mem, stack);
  }
  case InstrType::Lt: {
    if (stack.size() < 2) {
      throw runtime_error("Lt expects two arguments.");
    }
    Word32 r = stack.back();
    stack.pop_back();
    Word32 l = stack.back();
    stack.pop_back();
    int lInt = wordToInt(l);
    int rInt = wordToInt(r);
    bool result = lInt < rInt;
    stack.push_back(boolToWord(result));
    return State(pc + 1, mem, stack);
  }
  case InstrType::Eq: {
    if (stack.size() < 2) {
      throw runtime_error("Eq expects two arguments.");
    }
    Word32 r = stack.back();
    stack.pop_back();
    Word32 l = stack.back();
    stack.pop_back();
    int lInt = wordToInt(l);
    int rInt = wordToInt(r);
    bool result = lInt == rInt;
    stack.push_back(boolToWord(result));
    return State(pc + 1, mem, stack);
  }
  case InstrType::Push: {
    if (stack.empty()) {
      throw runtime_error("Push expects one argument.");
    }
    Word32 w = stack.back();
    stack.pop_back();
    stack.push_back(w);
    return State(pc + 1, mem, stack);
  }
  case InstrType::Pop: {
    if (stack.empty()) {
      throw runtime_error("Pop expects one argument.");
    }
    stack.pop_back();
    return State(pc + 1, mem, stack);
  }
  case InstrType::Store: {
    if (stack.size() < 2) {
      throw runtime_error("Store expects two arguments.");
    }
    Word32 addr = stack.back();
    stack.pop_back();
    Word32 w = stack.back();
    stack.pop_back();
    mem[addr] = w;
    return State(pc + 1, mem, stack);
  }
  case InstrType::Load: {
    if (stack.empty()) {
      throw runtime_error("Load expects one argument.");
    }
    Word32 addr = stack.back();
    stack.pop_back();
    auto it = mem.find(addr);
    if (it != mem.end()) {
      stack.push_back(it->second);
      return State(pc + 1, mem, stack);
    } else {
      throw runtime_error("Nothing to load at address.");
    }
  }
  case InstrType::Read: {
    cout << "? ";
    cout.flush();
    string input;
    cin >> input;
    Word32 w = static_cast<Word32>(stoi(input));
    stack.push_back(w);
    return State(pc + 1, mem, stack);
  }
  case InstrType::Print: {
    if (stack.empty()) {
      throw runtime_error("Print expects one argument.");
    }
    Word32 w = stack.back();
    stack.pop_back();
    cout << w << endl;
    return State(pc + 1, mem, stack);
  }
  case InstrType::Swap: {
    if (stack.size() < 2) {
      throw runtime_error("Swap expects two arguments.");
    }
    Word32 y = stack.back();
    stack.pop_back();
    Word32 x = stack.back();
    stack.pop_back();
    stack.push_back(y);
    stack.push_back(x);
    return State(pc + 1, mem, stack);
  }
  case InstrType::Dup: {
    if (stack.empty()) {
      throw runtime_error("Dup expects one argument.");
    }
    Word32 w = stack.back();
    stack.push_back(w);
    return State(pc + 1, mem, stack);
  }
  case InstrType::Over: {
    if (stack.empty()) {
      throw runtime_error("Over expects one argument.");
    }
    Word32 w = stack.back();
    stack.push_back(w);
    return State(pc + 1, mem, stack);
  }
  case InstrType::RotL: {
    if (stack.empty()) {
      throw runtime_error("RotL expects one argument.");
    }
    Word32 w = stack.back();
    stack.pop_back();
    stack.insert(stack.begin(), w);
    return State(pc + 1, mem, stack);
  }
  case InstrType::Done: {
    throw runtime_error("No state transition for Done!!");
  }
  }
}
