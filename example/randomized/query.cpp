#include <unordered_map>
#include <unordered_set>

#include "../../libgymbo/compiler.h"
#include "../../libgymbo/psymbolic.h"

char *user_input;
int max_depth = 65536;
int maxSAT = 65536;
int maxUNSAT = 65536;
int verbose_level = 2;
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

std::string mem2string(const gymbo::Mem &mem) {
    std::string expr = "Concrete Memory: {";
    float tmp_word;
    for (auto &t : mem) {
        tmp_word = gymbo::wordToFloat(t.second);
        if (gymbo::is_integer(tmp_word)) {
            expr += ("var_" + std::to_string(((int)t.first)) + ": " +
                     std::to_string((int)tmp_word)) +
                    ", ";
        } else {
            expr += ("var_" + std::to_string(((int)t.first)) + ": " +
                     std::to_string(tmp_word)) +
                    ", ";
        }
    }
    expr += "}";
    return expr;
}

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
        {1, gymbo::DiscreteUniformDist(1, 3)},
        {3, gymbo::DiscreteUniformDist(1, 3)}};

    std::vector<std::vector<int>> val_candidates;
    for (auto &vd : var2dist) {
        val_candidates.emplace_back(vd.second.vals);
    }
    std::vector<std::vector<int>> D = gymbo::cartesianProduct(val_candidates);

    printf("Start Symbolic Execution...\n");
    gymbo::PSExecutor executor(optimizer, maxSAT, maxUNSAT, max_num_trials,
                               ignore_memory, use_dpll, verbose_level);
    executor.register_random_var(1);
    executor.register_random_var(3);
    executor.run(prg, target_pcs, init, max_depth);
    printf("---------------------------\n");

    printf("Result Summary\n");
    printf("#Loops Spent for Gradient Descent: %d\n", optimizer.num_used_itr);

    int num_unique_path_constraints = executor.constraints_cache.size();
    int num_unique_final_states = executor.prob_constraints_table.size();

    std::unordered_map<int, float> params;
    gymbo::Sym q(gymbo::SymType::SCon, gymbo::FloatToWord(1.0));

    if (num_unique_path_constraints == 0) {
        printf("No Path Constraints Found\n");
    } else {
        printf("#Total Final States: %d\n", num_unique_final_states);
        if (verbose_level >= 0) {
            printf("List of Final States\n");
            for (auto &cc : executor.prob_constraints_table) {
                for (auto &ccv : cc.second) {
                    printf("pc=%d: prob=%s, %s, constraints=%s\n", cc.first,
                           std::get<2>(ccv)
                               .query(gymbo::SymType::SEq, q, var2dist, D)
                               .toString(true)
                               .c_str(),
                           mem2string(std::get<1>(ccv)).c_str(),
                           std::get<0>(ccv).toString(true).c_str());
                }
            }
        }
    }
}
