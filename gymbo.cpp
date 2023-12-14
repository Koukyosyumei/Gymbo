/**
 * @file gymbo.cpp
 * @brief CLT tool for gradient-based symbolic execution
 * @author Hideaki Takahashi
 */

#include <unistd.h>

#include <chrono>
#include <unordered_map>
#include <unordered_set>

#include "libgymbo/compiler.h"
#include "libgymbo/symbolic.h"

char *user_input;
int max_depth = 65536;
int maxSAT = 65536;
int maxUNSAT = 65536;
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
bool init_param_uniform_int = true;

void parse_args(int argc, char *argv[]) {
    int opt;
    user_input = argv[1];
    while ((opt = getopt(argc, argv, "d:v:i:a:e:t:l:h:s:gmrp")) != -1) {
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
            case 'r':
                init_param_uniform_int = false;
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
                    "param_high], [-s: seed], [-g off_sign_grad], [-r "
                    "off_init_param_uniform_int], [-m: "
                    "ignore_memory] "
                    "...\n",
                    argv[0]);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();

    std::unordered_map<std::string, int> var_counter;
    std::vector<gymbo::Node *> code;
    // std::unordered_map<int, std::string> id2varname;

    gymbo::Prog prg;
    gymbo::GDOptimizer optimizer(num_itrs, step_size, eps, param_low,
                                 param_high, sign_grad, init_param_uniform_int,
                                 seed);
    gymbo::SymState init;
    std::unordered_set<int> target_pcs;

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

    gymbo::SExecutor executor(prg, optimizer, target_pcs, maxSAT, maxUNSAT,
                              max_num_trials, ignore_memory, use_dpll,
                              verbose_level);

    printf("Start Symbolic Execution...\n");
    executor.run(init, max_depth);
    printf("---------------------------\n");

    end = std::chrono::system_clock::now();
    float elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    printf("Search time is complete %f [ms] \n", elapsed);

    printf("Result Summary\n");
    printf("#Loops Spent for Gradient Descent: %d\n", optimizer.num_used_itr);
    int num_unique_path_constraints = executor.constraints_cache.size();
    int num_sat = 0;
    int num_unsat = 0;
    for (auto &cc : executor.constraints_cache) {
        if (cc.second.first) {
            num_sat++;
        } else {
            num_unsat++;
        }
    }
    if (num_unique_path_constraints == 0) {
        printf("No Path Constraints Found\n");
    } else {
        printf("#Total Path Constraints: %d\n", num_unique_path_constraints);
        printf("#SAT: %d\n", num_sat);
        printf("#UNSAT: %d\n", num_unsat);

        if (verbose_level >= 0) {
            // for (auto vc : var_counter) {
            //    id2varname.emplace(vc.second, vc.first);
            // }

            printf("List of SAT Path Constraints\n----\n");
            for (auto &cc : executor.constraints_cache) {
                if (cc.second.first) {
                    printf("%s", cc.first.c_str());
                    printf("SAT Params: {");
                    for (auto p : cc.second.second) {
                        if (gymbo::is_integer(p.second)) {
                            printf("var_%d:%d, ", p.first, (int)p.second);
                        } else {
                            printf("var_%d:%f, ", p.first, p.second);
                        }
                    }
                    printf("}\n");
                    printf("----\n");
                }
            }

            printf("\nList of UNSAT Path Constraints\n");
            for (auto &cc : executor.constraints_cache) {
                if (!cc.second.first) {
                    printf("%s", cc.first.c_str());
                    printf("----\n");
                }
            }
        }
    }
}
