#include <unistd.h>
#include <unordered_map>

#include "include/compiler.h"
#include "include/parser.h"
#include "include/tokenizer.h"
#include "include/type.h"

char *user_input;
int max_depth = 256;
bool print_prg = false;

void parse_args(int argc, char *argv[]) {
  int opt;
  user_input = argv[1];
  while ((opt = getopt(argc, argv, "d:p")) != -1) {
    switch (opt) {
    case 'd':
      max_depth = atoi(optarg);
      break;
    case 'p':
      print_prg = true;
      break;
    default:
      printf("unknown parameter %s is specified", optarg);
      printf("Usage: %s [-d] ...\n", argv[0]);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  Node *node;
  std::vector<Node *> code;
  Prog prg;
  SymState init;
  std::unordered_map<std::string, std::pair<bool, std::unordered_map<int, int>>>
      cache_constraints;

  Token *token = tokenize(user_input);
  program(token, user_input, code);

  for (int i = 0; i < code.size(); i++) {
    if (code[i] != nullptr) {
      gen(code[i], prg);
    } else {
      prg.emplace_back(Instr(InstrType::Done));
    }
  }

  if (print_prg) {
    printf("Compiled Stack Machine...\n\n");
    for (int j = 0; j < prg.size(); j++) {
      prg[j].print();
    }
    printf("-------------------------\n\n");
  }

  printf("Start Symbolic Execution...\n\n");
  Trace trace = symRun(prg, init, cache_constraints, max_depth);
  printf("---------------------------\n");
}
