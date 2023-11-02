#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "utils.h"

using Word32 = uint32_t;

enum class InstrType {
  Add,
  JmpIf,
  Jmp,
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

class Instr {
public:
  InstrType instr;
  Word32 word;

  Instr(InstrType instr) : instr(instr) {}
  Instr(InstrType instr, Word32 word) : instr(instr), word(word) {}

  void print() {
    switch (instr) {
    case (InstrType::Add): {
      printf("add\n");
      return;
    }
    case (InstrType::JmpIf): {
      printf("jmpIf\n");
      return;
    }
    case (InstrType::Jmp): {
      printf("jmp\n");
      return;
    }
    case (InstrType::Lt): {
      printf("lt\n");
      return;
    }
    case (InstrType::Load): {
      printf("load\n");
      return;
    }
    case (InstrType::Read): {
      printf("read\n");
      return;
    }
    case (InstrType::Done): {
      printf("ret\n");
      return;
    }
    case (InstrType::Nop): {
      printf("nop\n");
      return;
    }
    case (InstrType::Swap): {
      printf("swap\n");
      return;
    }
    case (InstrType::Store): {
      printf("store\n");
      return;
    }
    case (InstrType::Push): {
      printf("push %d\n", word);
      return;
    }
    }
  }
};

using Prog = std::vector<Instr>;
using Mem = std::unordered_map<Word32, Word32>;

struct State {
  int pc;
  Mem mem;
  std::vector<Word32> stack;

  State(int pc, Mem mem, std::vector<Word32> stack)
      : pc(pc), mem(mem), stack(stack) {}
};

struct Grad {
  std::unordered_map<int, int> val;
  Grad(std::unordered_map<int, int> val) : val(val) {}
  Grad operator+(const Grad &other) {
    std::unordered_map<int, int> result = val;
    for (auto &r : result) {
      if (other.val.find(r.first) != other.val.end()) {
        result.at(r.first) += other.val.at(r.first);
      }
    }
    for (auto &r : other.val) {
      if (result.find(r.first) == result.end()) {
        result.emplace(std::make_pair(r.first, r.second));
      }
    }
    return Grad(result);
  }
  Grad operator+(int w) {
    std::unordered_map<int, int> result = val;
    for (auto &r : result) {
      result.at(r.first) += w;
    }
    return Grad(result);
  }
  Grad operator-(const Grad &other) {
    std::unordered_map<int, int> result = val;
    for (auto &r : result) {
      if (other.val.find(r.first) != other.val.end()) {
        result.at(r.first) -= other.val.at(r.first);
      }
    }
    for (auto &r : other.val) {
      if (result.find(r.first) == result.end()) {
        result.emplace(std::make_pair(r.first, r.second));
      }
    }
    return Grad(result);
  }
  Grad operator*(int w) {
    std::unordered_map<int, int> result = val;
    for (auto &r : result) {
      result.at(r.first) *= w;
    }
    return Grad(result);
  }
  Grad abs() {
    std::unordered_map<int, int> result = val;
    for (auto &r : result) {
      result.emplace(std::make_pair(r.first, std::abs(r.second)));
    }
    return Grad(result);
  }
};

enum class SymType { SAdd, SEq, SNot, SOr, SCon, SAnd, SLt, SAny };

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

  void gather_var_ids(std::unordered_set<int> &result) {
    switch (symtype) {
    case (SymType::SAdd): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SAny): {
      result.emplace(var_idx);
    }
    case (SymType::SEq): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SNot): {
      left->gather_var_ids(result);
      return;
    }
    case (SymType::SAnd): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SLt): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    default:
      return;
    }
  }

  int eval(const std::unordered_map<int, int> &cvals) {
    switch (symtype) {
    case (SymType::SAdd): {
      return left->eval(cvals) + right->eval(cvals);
    }
    case (SymType::SCon): {
      return wordToSignedInt(word);
    }
    case (SymType::SAny): {
      return cvals.at(var_idx);
    }
    case (SymType::SEq): {
      return std::abs(left->eval(cvals) - right->eval(cvals));
    }
    case (SymType::SNot): {
      return left->eval(cvals) * (-1) + 1;
    }
    case (SymType::SAnd): {
      return left->eval(cvals) + right->eval(cvals);
    }
    case (SymType::SLt): {
      return left->eval(cvals) - right->eval(cvals) + 1;
    }
    }
    return 0;
  }

  Grad grad() {
    switch (symtype) {
    case (SymType::SAdd): {
      return left->grad() + right->grad();
    }
    case (SymType::SCon): {
      return Grad({});
    }
    case (SymType::SAny): {
      std::unordered_map<int, int> tmp = {{var_idx, 1}};
      return Grad(tmp);
    }
    case (SymType::SEq): {
      return (left->grad() - right->grad()).abs();
    }
    case (SymType::SNot): {
      return left->grad() * (-1);
    }
    case (SymType::SAnd): {
      return left->grad() + right->grad();
    }
    case (SymType::SLt): {
      return left->grad() - right->grad();
    }
    }
    return Grad({});
  }

  std::string toString() {
    std::string result = "";
    switch (symtype) {
    case (SymType::SAdd): {
      result = "(" + left->toString() + "+" + right->toString() + ")";
      break;
    }
    case (SymType::SCon): {
      result += std::to_string(wordToSignedInt(word));
      break;
    }
    case (SymType::SAny): {
      result += "var_" + std::to_string(var_idx);
      break;
    }
    case (SymType::SEq): {
      result += left->toString() + " == " + right->toString();
      break;
    }
    case (SymType::SNot): {
      result += "!(" + left->toString() + ")";
      break;
    }
    case (SymType::SAnd): {
      result += left->toString() + " && " + right->toString();
      break;
    }
    case (SymType::SOr): {
      result += left->toString() + " || " + right->toString();
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
    printf("Stack: [");
    LLNode<Sym> *tmp = symbolic_stack.head;
    while (tmp != NULL) {
      printf("%s, ", tmp->data.toString().c_str());
      tmp = tmp->next;
    }
    printf("]\n");

    printf("Memory: {");
    for (auto t : mem) {
      printf("var_%d: %d, ", t.first, t.second);
    }
    printf("}\n");

    printf("Path Constraints: ");
    for (Sym &sym : path_constraints) {
      printf("%s and ", sym.toString().c_str());
    }
    printf(" 1\n");
  }
};

struct Trace {
  SymState data;
  std::vector<Trace> children;

  Trace(SymState data, std::vector<Trace> children)
      : data(data), children(children) {}

  void print(int indent_cnt = 0) {
    printf("PC: %d\n", data.pc);
    data.print();
    for (Trace &trace : children) {
      trace.print();
    }
  }
};
