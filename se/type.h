#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "utils.h"

using Word32 = uint32_t;

enum InstrType : uint8_t {
  Add,
  JmpIf,
  And,
  Or,
  Not,
  Lt,
  Eq,
  Push,
  Store,
  Load,
  Pop,
  Read,
  Print,
  Swap,
  Dup,
  Over,
  RotL,
  Done
};

// Define the Instr enum
class Instr {
public:
  InstrType instr;
  Word32 word;

  Instr(InstrType instr) : instr(instr) {}
  Instr(InstrType instr, Word32 word) : instr(instr), word(word) {}
};

// Define the Prog type
using Prog = std::vector<Instr>;

// Define the Mem type
using Mem = std::map<Word32, Word32>;

// Define the State type
// using State = std::tuple<int, Mem, std::vector<Word32>>;

struct State {
  int pc;
  Mem mem;
  std::vector<Word32> stack;

  State(int pc, Mem mem, std::vector<Word32> stack)
      : pc(pc), mem(mem), stack(stack) {}
};

enum class SymType { SAdd, SEq, SNot, SOr, SCon, SAnd, SLt, SAny };

// Define the Sym struct
struct Sym {
  SymType symtype;
  Sym *left;
  Sym *right;
  Word32 word;
  int var_idx;

  Sym() {}
  Sym(SymType symtype, Sym *left) : symtype(symtype), left(left) {}
  Sym(SymType symtype, Sym *left, Sym *right)
      : symtype(symtype), left(left), right(right) {}

  Sym(SymType symtype, int val) : symtype(symtype) {
    if (symtype == SymType::SAny) {
      var_idx = val;
    } else {
      word = val;
    }
  }

  std::string toString() {
    std::string result;
    switch (symtype) {
    case (SymType::SAdd): {
      result = "(" + left->toString() + "+" + right->toString() + ")";
    }
    case (SymType::SCon): {
      result += wordToSignedInt(word);
    }
    case (SymType::SAny): {
      result += std::to_string(var_idx);
    }
    case (SymType::SEq): {
      result += left->toString() + " = " + right->toString();
    }
    case (SymType::SNot): {
      result += "~(" + left->toString() + ")";
    }
    case (SymType::SAnd): {
      result += left->toString() + " and " + right->toString();
    }
    case (SymType::SOr): {
      result += left->toString() + " or " + right->toString();
    }
    case (SymType::SLt): {
      result += left->toString() + " < " + right->toString();
    }
    }
    return result;
  }
};

// Define the SymState type

struct SymState {
  int pc;
  int var_cnt;
  std::map<Word32, Sym> mem;
  Linkedlist<Sym> symbolic_stack;
  std::vector<Sym> path_constraints;

  SymState() : pc(0), var_cnt(0){};
  SymState(int pc, int varcnt, std::map<Word32, Sym> mem,
           Linkedlist<Sym> symbolic_stack, std::vector<Sym> path_constraints)
      : pc(pc), mem(mem), symbolic_stack(symbolic_stack),
        path_constraints(path_constraints) {}

  void print() {
    std::cout << "Stack: [";
    LLNode<Sym> *tmp = symbolic_stack.head;
    while (tmp != NULL) {
      std::cout << tmp->data.toString() << ", ";
    }
    std::cout << "]\n";

    std::cout << "Path Constraints: ";
    for (Sym &sym : path_constraints) {
      sym.toString();
      std::cout << " and ";
    }
    std::cout << std::endl;
  }
};

// Define a tree structure for SymState
struct Trace {
  SymState data;
  std::vector<Trace> children;

  Trace(SymState data, std::vector<Trace> children)
      : data(data), children(children) {}
};
