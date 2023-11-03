#include <unistd.h>

#include "libgymbo/compiler.h"
#include "libgymbo/gd.h"
#include "libgymbo/parser.h"
#include "libgymbo/tokenizer.h"
#include "libgymbo/type.h"

char *user_input;
int max_depth = 256;
int verbose_level = 1;
int num_itrs = 100;
int step_size = 1;

void parse_args(int argc, char *argv[]) {
  int opt;
  user_input = argv[1];
  while ((opt = getopt(argc, argv, "d:v:i:a:")) != -1) {
    switch (opt) {
    case 'd':
      max_depth = atoi(optarg);
      break;
    case 'v':
      verbose_level = atoi(optarg);
      break;
    case 'i':
      num_itrs = atoi(optarg);
      break;
    case 'a':
      step_size = atoi(optarg);
      break;
    default:
      printf("unknown parameter %s is specified", optarg);
      printf("Usage: %s [-d], [-v] ...\n", argv[0]);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  Node *node;
  std::vector<Node *> code;
  Prog prg;
  GDOptimizer optimizer(num_itrs, step_size);
  SymState init;
  PathConstraintsTable cache_constraints;

  printf("Compiling the input program...\n");
  Token *token = tokenize(user_input);
  generate_ast(token, user_input, code);
  compile_ast(code, prg);

  if (verbose_level >= 2) {
    printf("...Compiled Stack Machine...\n");
    for (int j = 0; j < prg.size(); j++) {
      prg[j].print();
    }
    printf("----------------------------\n");
  }

  printf("Start Symbolic Execution...\n");
  Trace trace =
      symRun(prg, optimizer, init, cache_constraints, max_depth, verbose_level);
  printf("---------------------------\n");

  printf("Result Summary\n");
  int num_unique_path_constraints = cache_constraints.size();
  int num_sat = 0;
  int num_unsat = 0;
  for (auto &cc : cache_constraints) {
    if (cc.second.first) {
      num_sat++;
    } else {
      num_unsat++;
    }
  }
  if (num_unique_path_constraints == 0) {
    printf("No Path Constraints Found");
  } else {
    printf("#Total Path Constraints: %d\n", num_unique_path_constraints);
    printf("#SAT: %d\n", num_sat);
    printf("#UNSAT: %d\n", num_unsat);
  }
}
