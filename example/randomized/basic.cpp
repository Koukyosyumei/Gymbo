#include <unordered_map>

#include "../../libgymbo/compiler.h"
#include "../../libgymbo/psymbolic.h"

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

int main(int argc, char *argv[]) {
    user_input = argv[1];

    std::unordered_map<std::string, int> var_counter;
    std::vector<gymbo::Node *> code;
    // std::unordered_map<int, std::string> id2varname;

    gymbo::Prog prg;
    gymbo::GDOptimizer optimizer(num_itrs, step_size, eps, param_low,
                                 param_high, sign_grad, init_param_uniform_int,
                                 seed);
    gymbo::SymState init;
    gymbo::PathConstraintsTable cache_constraints;
    gymbo::ProbPathConstraintsTable probabilistic_constraints;
    std::unordered_set<int> target_pcs;

    printf("Compiling the input program...\n");
    gymbo::Token *token = gymbo::tokenize(user_input, var_counter);
    gymbo::generate_ast(token, user_input, code);
    gymbo::compile_ast(code, prg);

    for (auto &vc : var_counter) {
        printf("%s:%d\n", vc.first.c_str(), vc.second);
    }
    printf("---\n");

    std::unordered_map<int, gymbo::DiscreteDist> var2dist = {
        {0, gymbo::DiscreteUniformDist(1, 3)},
        {1, gymbo::DiscreteUniformDist(1, 3)}};

    std::vector<std::vector<int>> val_candidates;
    for (auto &vd : var2dist) {
        val_candidates.emplace_back(vd.second.vals);
    }
    std::vector<std::vector<int>> D = gymbo::cartesianProduct(val_candidates);

    printf("Start Symbolic Execution...\n");
    gymbo::run_pgymbo(prg, var2dist, D, optimizer, init, target_pcs,
                      cache_constraints, probabilistic_constraints, max_depth,
                      maxSAT, maxUNSAT, max_num_trials, ignore_memory, use_dpll,
                      verbose_level);
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
        printf("No Path Constraints Found\n");
    } else {
        printf("#Total Path Constraints: %d\n", num_unique_path_constraints);
        printf("#SAT: %d\n", num_sat);
        printf("#UNSAT: %d\n", num_unsat);

        if (verbose_level >= 0) {
            printf("List of Path Constraints\n");
            for (auto &cc : probabilistic_constraints) {
                for (auto &ccv : cc.second) {
                    printf("pc=%d: %s, prob=%f\n", cc.first,
                           std::get<0>(ccv).toString(true).c_str(),
                           std::get<2>(ccv));
                }
            }
        }
    }

    printf("Done\n");
}
