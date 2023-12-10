#include <string>
#include <unordered_map>

#include "../../libgymbo/compiler.h"
#include "../../libgymbo/psymbolic.h"

char *user_input;
int max_depth = 65536;
int maxSAT = 65536;
int maxUNSAT = 65536;
int verbose_level = -2;
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
        tmp_word = wordToFloat(t.second);
        if (is_integer(tmp_word)) {
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

std::string param2string(const std::unordered_map<int, float> &param) {
    std::string expr = "Params: {";
    for (auto &p : param) {
        expr += ("var_" + std::to_string(p.first) + ": " +
                 std::to_string((int)p.second) + ", ");
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
    std::vector<std::vector<int>> D = cartesianProduct(val_candidates);

    std::vector<int> doow_switch_candidates = {0, 1};

    for (int door_switch : doow_switch_candidates) {
        gymbo::SymState init;
        gymbo::PathConstraintsTable cache_constraints;
        gymbo::ProbPathConstraintsTable probabilistic_constraints;
        init.set_concrete_val(var_counter["door_switch"], door_switch);
        gymbo::run_pgymbo(prg, var2dist, D, optimizer, init, target_pcs,
                          cache_constraints, probabilistic_constraints,
                          max_depth, maxSAT, maxUNSAT, max_num_trials,
                          ignore_memory, use_dpll, verbose_level);

        int num_unique_path_constraints = cache_constraints.size();
        int num_unique_final_states = probabilistic_constraints.size();

        std::unordered_map<int, float> params;

        printf("\nResult Summary\n");
        if (num_unique_path_constraints == 0) {
            printf("No Path Constraints Found\n");
        } else {
            printf("\n#Total Final States: %d\n", num_unique_final_states);
            printf("List of Final States\n");
            for (auto &cc : probabilistic_constraints) {
                for (auto &ccv : cc.second) {
                    float p = std::get<2>(ccv).eval(params, eps, var2dist, D);
                    if (p > 0.0f) {
                        printf("pc=%d: prob=%f, %s, constraints=%s\n", cc.first,
                               p, mem2string(std::get<1>(ccv)).c_str(),
                               std::get<0>(ccv).toString(true).c_str());
                    }
                }
            }
        }
    }
}