#include <unistd.h>

#include <unordered_map>

#include "libgymbo/compiler.h"
#include "libgymbo/gd.h"
#include "libgymbo/parser.h"
#include "libgymbo/tokenizer.h"
#include "libgymbo/type.h"

char *user_input;
int max_depth = 65536;
int verbose_level = 1;
int num_itrs = 100;
float step_size = 1.0f;
float eps = 1.0f;
int max_num_trials = 10;
int param_low = -10;
int param_high = 10;
int seed = 42;
bool sign_grad = true;
bool ignore_memory = false;
bool use_dpll = false;

void parse_args(int argc, char *argv[]) {
    int opt;
    user_input = argv[1];
    while ((opt = getopt(argc, argv, "d:v:i:a:e:t:l:h:s:gmp")) != -1) {
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
                step_size = atof(optarg);
                break;
            case 'e':
                eps = atof(optarg);
                break;
            case 't':
                max_num_trials = atoi(optarg);
                break;
            case 'l':
                param_low = atoi(optarg);
                break;
            case 'h':
                param_high = atoi(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'g':
                sign_grad = false;
                break;
            case 'm':
                ignore_memory = true;
                break;
            case 'p':
                use_dpll = true;
                break;
            default:
                printf("unknown parameter %s is specified", optarg);
                printf(
                    "Usage: %s [-d: max_depth], [-v: verbose level], [-i: "
                    "num_itrs], "
                    "[-a: step_size], [-t: max_num_trials], [-l: param_low], "
                    "[-h: "
                    "param_high], [-s: seed], [-g off_sign_grad], [-m: "
                    "ignore_memory] "
                    "...\n",
                    argv[0]);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    std::unordered_map<std::string, int> var_counter;
    std::vector<gymbo::Node *> code;
    std::unordered_map<int, std::string> id2varname;
    
    gymbo::Prog prg;
    gymbo::GDOptimizer optimizer(num_itrs, step_size, eps, param_low,
                                 param_high, sign_grad, seed);
    gymbo::SymState init;
    gymbo::PathConstraintsTable cache_constraints;

    printf("Compiling the input program...\n");
    gymbo::Token *token = gymbo::tokenize(user_input, var_counter);
    gymbo::generate_ast(token, user_input, code);
    gymbo::compile_ast(code, prg);

    if (verbose_level >= 3) {
        printf("...Compiled Stack Machine...\n");
        for (int j = 0; j < prg.size(); j++) {
            prg[j].print();
        }
        printf("----------------------------\n");
    }

    if (verbose_level >= 1) {
        printf("...Variables\n");
        for (auto &kv : var_counter) {
            printf("%s: %d\n", kv.first.c_str(), kv.second);
        }
    }

    printf("Start Symbolic Execution...\n");
    gymbo::run_gymbo(prg, optimizer, init, cache_constraints, max_depth,
                     max_num_trials, ignore_memory, use_dpll, verbose_level);
    printf("---------------------------\n");

    printf("Result Summary\n");
    printf("#Loops Spent for Gradient Descent: %d\n", optimizer.num_used_itr);
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

        if (verbose_level >= 0) {
            for (auto vc : var_counter) {
                id2varname.emplace(vc.second, vc.first);
            }

            printf("List of SAT Path Constraints\n");
            for (auto &cc : cache_constraints) {
                if (cc.second.first) {
                    printf("# %s\n", cc.first.c_str());
                    printf("   {");
                    for (auto p : cc.second.second) {
                        if (is_integer(p.second)) {
                            printf("%s:%d, ", id2varname[p.first].c_str(),
                                   (int)p.second);
                        } else {
                            printf("%s:%f, ", id2varname[p.first].c_str(),
                                   p.second);
                        }
                    }
                    printf("}\n");
                }
            }

            printf("List of UNSAT Path Constraints\n");
            for (auto &cc : cache_constraints) {
                if (!cc.second.first) {
                    printf("# %s\n", cc.first.c_str());
                }
            }
        }
    }
}
