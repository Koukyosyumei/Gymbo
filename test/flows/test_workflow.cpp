#include "../../libgymbo/compiler.h"
#include "../../libgymbo/symbolic.h"
#include "gtest/gtest.h"

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

TEST(GymboWorlflowTest, Block) {
    std::string code_str =
        "if (a > 2) {\n"
        "    b = 1;\n"
        "    \n"
        "    if (b == 4) {\n"
        "        c = 3;\n"
        "    }\n"
        "    \n"
        "    if (b == 3) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        c = 1;\n"
        "    }\n"
        "}\n"
        "\n"
        "if (c == 1)\n"
        "    return 2;";
    char *user_input = const_cast<char *>(code_str.c_str());

    std::unordered_map<std::string, int> var_counter;
    std::vector<gymbo::Node *> code;

    gymbo::Prog prg;
    gymbo::GDOptimizer optimizer(num_itrs, step_size, eps, param_low,
                                 param_high, sign_grad, init_param_uniform_int,
                                 seed);
    gymbo::SymState init;
    std::unordered_set<int> target_pcs;

    gymbo::Token *token = gymbo::tokenize(user_input, var_counter);
    gymbo::generate_ast(token, user_input, code);
    gymbo::compile_ast(code, prg);

    gymbo::SExecutor executor(optimizer, maxSAT, maxUNSAT, max_num_trials,
                              ignore_memory, use_dpll, verbose_level);
    executor.run(prg, target_pcs, init, max_depth);

    int num_sat = 0;
    int num_unsat = 0;
    for (auto &cc : executor.constraints_cache) {
        if (cc.second.first) {
            num_sat++;
        } else {
            num_unsat++;
        }
    }

    ASSERT_EQ(num_sat, 7);
    ASSERT_EQ(num_unsat, 3);
}
