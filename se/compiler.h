#include "parser.h"
#include "symbolic.h"
#include "tokenizer.h"
#include "type.h"

inline void gen_lval(Node *node, Prog &prg) {
  if (node->kind != ND_LVAR) {
    char em[] = "lvar is not a variable";
    error(em);
  }
  prg.emplace_back(Instr(InstrType::Read));
  // prg.emplace_back(Instr(InstrType::Push, node->offset));
}

inline void gen(Node *node, Prog &prg) {
  if (node->kind == ND_RETURN) {
    prg.emplace_back(Instr(InstrType::Done));
    return;
  }

  switch (node->kind) {
  case ND_IF: {
    gen(node->cond, prg);
    Prog if_true_prg;
    gen(node->lhs, if_true_prg);
    prg.emplace_back(
        Instr(InstrType::Push, 4)); // relative address to true clause
    prg.emplace_back(Instr(InstrType::Swap));
    prg.emplace_back(Instr(InstrType::JmpIf));
    prg.emplace_back(Instr(InstrType::Nop)); // currently not support else
    prg.insert(prg.end(), if_true_prg.begin(), if_true_prg.end());
    return;
  }
  case ND_NUM: {
    prg.emplace_back(Instr(InstrType::Push, node->val));
    return;
  }
  case ND_LVAR: {
    gen_lval(node, prg);
    return;
  }
  case ND_ASSIGN: {
    gen_lval(node->lhs, prg);
    gen(node->rhs, prg);
    prg.emplace_back(Instr(InstrType::Store));
    return;
  }
  }

  gen(node->lhs, prg);
  gen(node->rhs, prg);

  switch (node->kind) {
  case ND_ADD:
    prg.emplace_back(Instr(InstrType::Add));
    return;
  case ND_EQ:
    prg.emplace_back(Instr(InstrType::Eq));
    return;
  case ND_LT:
    prg.emplace_back(Instr(InstrType::Lt));
    return;
  }

  char em[] = "Node is not supported";
  error(em);
}
