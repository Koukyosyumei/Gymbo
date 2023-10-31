#include "compiler.h"
#include "parser.h"
#include "tokenizer.h"
#include <iostream>

int main() {
  Node *node;
  std::vector<Node *> code;
  Prog prg;

  char user_input[] = "1 + 1;";
  std::cout << 1 << std::endl;
  Token *token = tokenize(user_input);

  std::cout << 2 << std::endl;
  program(token, user_input, code);
  std::cout << 3 << std::endl;

  std::cout << code.size() << std::endl;

  for (int i = 0; i < code.size(); i++) {
    std::cout << code[i]->kind << std::endl;
    gen(code[i], prg);
  }
  std::cout << 4 << std::endl;

  std::cout << " .. " << prg.size() << std::endl;

  for (int j = 0; j < prg.size(); j++) {
    prg[j].print();
  }

  std::cout << 5 << std::endl;
}
