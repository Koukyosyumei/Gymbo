#include "compiler.h"
#include "parser.h"
#include "tokenizer.h"
#include "type.h"
#include <iostream>

int main() {
  // char user_input[] = "1 + 1;";
  // char user_input[] = "a = 1; return a;";
  // char user_input[] = "if (a < 2) return a; if (b == 3) return b;";
  char user_input[] = "if (a < 2) 1 + 1;";

  Node *node;
  std::vector<Node *> code;
  Prog prg;
  SymState init;

  Token *token = tokenize(user_input);
  program(token, user_input, code);

  for (int i = 0; i < code.size(); i++) {
    if (code[i] != nullptr) {
      gen(code[i], prg);
    } else {
      prg.emplace_back(Instr(InstrType::Done));
    }
  }

  for (int j = 0; j < prg.size(); j++) {
    prg[j].print();
  }

  Trace trace = symRun(32, prg, init);
  std::cout << "Complete" << std::endl;
}
