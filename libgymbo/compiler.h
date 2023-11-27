#include "parser.h"
#include "symbolic.h"
#include "tokenizer.h"
#include "type.h"
#include "utils.h"

namespace gymbo {

/**
 * Generates virtual instructions for a given AST node, representing the
 * left-hand side of a variable assignment expression.
 *
 * @param node The AST node representing the left-hand side of a variable
 * assignment expression.
 * @param prg The virtual program to append the generated instructions to.
 */
inline void gen_lval(Node *node, Prog &prg) {
  if (node->kind != ND_LVAR) {
    char em[] = "lvar is not a variable";
    error(em);
  }
  prg.emplace_back(Instr(InstrType::Push, node->offset));
}

/**
 * Generates virtual instructions for a given AST node.
 *
 * @param node The AST node to generate LLVM instructions for.
 * @param prg The virtual program to append the generated instructions to.
 */
inline void gen(Node *node, Prog &prg) {

  switch (node->kind) {
  case (ND_RETURN): {
    prg.emplace_back(Instr(InstrType::Done));
    return;
  }
  case (ND_BLOCK): {
    for (Node *b : node->blocks) {
      gen(b, prg);
    }
    return;
  }
  case ND_IF: {
    gen(node->cond, prg);
    Prog then_prg, els_prg;
    gen(node->then, then_prg);

    if (node->els != nullptr) {
      gen(node->els, els_prg);
    } else {
      els_prg.emplace_back(Instr(InstrType::Nop));
    }
    els_prg.emplace_back(Instr(InstrType::Push, 1 + then_prg.size()));
    els_prg.emplace_back(Instr(InstrType::Jmp));

    prg.emplace_back(Instr(InstrType::Push, 3 + els_prg.size()));
    prg.emplace_back(Instr(InstrType::Swap));
    prg.emplace_back(Instr(InstrType::JmpIf));
    prg.insert(prg.end(), els_prg.begin(), els_prg.end());
    prg.insert(prg.end(), then_prg.begin(), then_prg.end());
    return;
  }
  case ND_NUM: {
    prg.emplace_back(Instr(InstrType::Push, FloatToWord(node->val)));
    return;
  }
  case ND_LVAR: {
    gen_lval(node, prg);
    prg.emplace_back(Instr(InstrType::Load));
    return;
  }
  case ND_ASSIGN: {
    gen_lval(node->lhs, prg);
    prg.emplace_back(Instr(InstrType::Load));
    gen(node->rhs, prg);
    prg.emplace_back(Instr(InstrType::Swap));
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
  case ND_SUB:
    prg.emplace_back(Instr(InstrType::Sub));
    return;
  case ND_MUL:
    prg.emplace_back(Instr(InstrType::Mul));
    return;
  case ND_EQ:
    prg.emplace_back(Instr(InstrType::Eq));
    return;
  case ND_NE:
    prg.emplace_back(Instr(InstrType::Eq));
    prg.emplace_back(Instr(InstrType::Not));
    return;
  case ND_LT:
    prg.emplace_back(Instr(InstrType::Lt));
    return;
  case ND_LE:
    prg.emplace_back(Instr(InstrType::Le));
    return;
  case ND_AND:
    prg.emplace_back(Instr(InstrType::And));
    return;
  case ND_OR:
    prg.emplace_back(Instr(InstrType::Or));
    return;
  }

  char em[] = "Unsupported Node";
  error(em);
}

inline void compile_ast(std::vector<Node *> code, Prog &prg) {
  for (int i = 0; i < code.size(); i++) {
    if (code[i] != nullptr) {
      gen(code[i], prg);
    } else {
      prg.emplace_back(Instr(InstrType::Done));
    }
  }
}
} // namespace gymbo
