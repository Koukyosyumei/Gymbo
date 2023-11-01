#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.h"

using Word32 = uint32_t;

enum class InstrType {
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
  Done,
  Nop,
};

// Define the Instr enum
class Instr {
public:
  InstrType instr;
  Word32 word;

  Instr(InstrType instr) : instr(instr) {}
  Instr(InstrType instr, Word32 word) : instr(instr), word(word) {}

  void print() {
    switch (instr) {
    case (InstrType::Add): {
      printf("Add\n");
      return;
    }
    case (InstrType::JmpIf): {
      printf("JmpIf\n");
      return;
    }
    case (InstrType::Lt): {
      printf("Lt\n");
      return;
    }
    case (InstrType::Load): {
      printf("Load\n");
      return;
    }
    case (InstrType::Read): {
      printf("Read\n");
      return;
    }
    case (InstrType::Done): {
      printf("Done\n");
      return;
    }
    case (InstrType::Nop): {
      printf("Nop\n");
      return;
    }
    case (InstrType::Swap): {
      printf("Swap\n");
      return;
    }
    case (InstrType::Store): {
      printf("Store\n");
      return;
    }
    case (InstrType::Push): {
      printf("Push %d\n", word);
      return;
    }
    }
  }
};

// Define the Prog type
using Prog = std::vector<Instr>;

// Define the Mem type
using Mem = std::unordered_map<Word32, Word32>;

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
    std::string result = "";
    switch (symtype) {
    case (SymType::SAdd): {
      result = "(" + left->toString() + "+" + right->toString() + ")";
      break;
    }
    case (SymType::SCon): {
      // std::cout << "word: " << wordToSignedInt(word);
      result += std::to_string(wordToSignedInt(word));
      break;
    }
    case (SymType::SAny): {
      result += "var_" + std::to_string(var_idx);
      break;
    }
    case (SymType::SEq): {
      result += left->toString() + " = " + right->toString();
      break;
    }
    case (SymType::SNot): {
      result += "~(" + left->toString() + ")";
      break;
    }
    case (SymType::SAnd): {
      result += left->toString() + " and " + right->toString();
      break;
    }
    case (SymType::SOr): {
      result += left->toString() + " or " + right->toString();
      break;
    }
    case (SymType::SLt): {
      result += left->toString() + " < " + right->toString();
      break;
    }
    }
    return result;
  }
};

// Define the SymState type

struct SymState {
  int pc;
  int var_cnt;
  Mem mem;
  Linkedlist<Sym> symbolic_stack;
  std::vector<Sym> path_constraints;

  SymState() : pc(0), var_cnt(0){};
  SymState(int pc, int varcnt, Mem mem, Linkedlist<Sym> symbolic_stack,
           std::vector<Sym> path_constraints)
      : pc(pc), mem(mem), symbolic_stack(symbolic_stack),
        path_constraints(path_constraints) {}

  void print() {
    std::cout << "Stack: [";
    LLNode<Sym> *tmp = symbolic_stack.head;
    while (tmp != NULL) {
      std::cout << tmp->data.toString() << ", ";
      tmp = tmp->next;
    }
    std::cout << "]\n";

    std::cout << "Memory: {";
    for (auto t : mem) {
      std::cout << "var_" << t.first << ": " << t.second << ", ";
    }
    std::cout << "}\n";

    std::cout << "Path Constraints: ";
    for (Sym &sym : path_constraints) {
      std::cout << sym.toString();
      std::cout << " and ";
    }
    std::cout << " 1";
    std::cout << std::endl;
  }
};

// Define a tree structure for SymState
struct Trace {
  SymState data;
  std::vector<Trace> children;

  Trace(SymState data, std::vector<Trace> children)
      : data(data), children(children) {}

  void print(int indent_cnt = 0) {
    std::cout << "PC: " << std::to_string(data.pc) << std::endl;
    data.print();
    for (Trace &trace : children) {
      trace.print();
    }
  }
};
