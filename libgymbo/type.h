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

namespace gymbo {

using Word32 = uint32_t;

enum class InstrType {
  Add,
  Sub,
  Mul,
  JmpIf,
  Jmp,
  And,
  Or,
  Not,
  Lt,
  Le,
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
    case (InstrType::Sub): {
      printf("sub\n");
      return;
    }
    case (InstrType::Mul): {
      printf("mul\n");
      return;
    }
    case (InstrType::And): {
      printf("and\n");
      return;
    }
    case (InstrType::Or): {
      printf("or\n");
      return;
    }
    case (InstrType::Not): {
      printf("not\n");
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
    case (InstrType::Le): {
      printf("le\n");
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
    default: {
      printf("unknown\n");
    }
    }
  }
};

using Prog = std::vector<Instr>;
using Mem = std::unordered_map<Word32, Word32>;
using PathConstraintsTable =
    std::unordered_map<std::string,
                       std::pair<bool, std::unordered_map<int, int>>>;

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
        result.emplace(std::make_pair(r.first, -1 * r.second));
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

enum class SymType {
  SAdd,
  SSub,
  SMul,
  SEq,
  SNot,
  SOr,
  SCon,
  SAnd,
  SLt,
  SLe,
  SAny
};

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
    case (SymType::SSub): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SMul): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SAny): {
      result.emplace(var_idx);
      return;
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
    case (SymType::SOr): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SLt): {
      left->gather_var_ids(result);
      right->gather_var_ids(result);
      return;
    }
    case (SymType::SLe): {
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
    case (SymType::SSub): {
      return left->eval(cvals) - right->eval(cvals);
    }
    case (SymType::SMul): {
      return left->eval(cvals) * right->eval(cvals);
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
      return std::max(0, left->eval(cvals)) + std::max(0, right->eval(cvals));
    }
    case (SymType::SOr): {
      return std::max(0, left->eval(cvals)) * std::max(0, right->eval(cvals));
    }
    case (SymType::SLt): {
      return left->eval(cvals) - right->eval(cvals) + 1;
    }
    case (SymType::SLe): {
      return left->eval(cvals) - right->eval(cvals);
    }
    default: {
      return 0;
    }
    }
  }

  Grad grad(const std::unordered_map<int, int> &cvals) {
    switch (symtype) {
    case (SymType::SAdd): {
      return left->grad(cvals) + right->grad(cvals);
    }
    case (SymType::SSub): {
      return left->grad(cvals) - right->grad(cvals);
    }
    case (SymType::SMul): {
      return left->grad(cvals) * right->eval(cvals) +
             right->grad(cvals) * left->eval(cvals);
    }
    case (SymType::SCon): {
      return Grad({});
    }
    case (SymType::SAny): {
      std::unordered_map<int, int> tmp = {{var_idx, 1}};
      return Grad(tmp);
    }
    case (SymType::SEq): {
      int lv = left->eval(cvals);
      int rv = right->eval(cvals);
      Grad lg = left->grad(cvals);
      Grad rg = right->grad(cvals);
      if (lv == rv) {
        return Grad({});
      } else if (lv > rv) {
        return lg - rg;
      } else {
        return rg - lg;
      }
    }
    case (SymType::SNot): {
      return left->grad(cvals) * (-1);
    }
    case (SymType::SAnd): {
      int lv = left->eval(cvals);
      int rv = right->eval(cvals);
      Grad res({});
      if (lv > 0) {
        res = res + left->grad(cvals);
      }
      if (rv > 0) {
        res = res + right->grad(cvals);
      }
      return res;
    }
    case (SymType::SOr): {
      int lv = left->eval(cvals);
      int rv = right->eval(cvals);
      if (lv > 0 && rv > 0) {
        return left->grad(cvals) * rv + right->grad(cvals) * lv;
      }
      return Grad({});
    }
    case (SymType::SLt): {
      return left->grad(cvals) - right->grad(cvals);
    }
    case (SymType::SLe): {
      return left->grad(cvals) - right->grad(cvals);
    }
    default: {
      return Grad({});
    }
    }
  }

  std::string toString() {
    std::string result = "";
    switch (symtype) {
    case (SymType::SAdd): {
      result = "(" + left->toString() + "+" + right->toString() + ")";
      break;
    }
    case (SymType::SSub): {
      result = "(" + left->toString() + "-" + right->toString() + ")";
      break;
    }
    case (SymType::SMul): {
      result = "(" + left->toString() + "*" + right->toString() + ")";
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
    case (SymType::SLe): {
      result += left->toString() + " <= " + right->toString();
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
      printf("(%s) && ", sym.toString().c_str());
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
} // namespace gymbo
