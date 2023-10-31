#include "parser.h"
#include "symbolic.h"
#include "tokenizer.h"
#include "type.h"

inline void gen_lval(Node *node, Prog &prg) {
  if (node->kind != ND_LVAR) {
    char em[] = "lvar is not a variable";
    error(em);
  }
  // その変数の場所をstackにpushする
  prg.emplace_back(Instr(InstrType::Push, node->offset));
}

inline void gen(Node *node, Prog &prg) {
  if (node->kind == ND_RETURN) {
    prg.emplace_back(Instr(InstrType::Done));
    return;
  }

  switch (node->kind) {
  case ND_IF:
    gen(node->cond, prg);
    prg.emplace_back(Instr(InstrType::JmpIf));
    gen(node->lhs, prg);
    return;
  case ND_NUM:
    prg.emplace_back(Instr(InstrType::Push, node->val));
    return;
  case ND_LVAR:
    gen_lval(node, prg);
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs, prg);
    gen(node->rhs, prg);
    prg.emplace_back(Instr(InstrType::Store));
    return;
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
