#include "../../libgymbo/compiler.h"
#include "../../libgymbo/psymbolic.h"
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

TEST(GymboWorkflowTest, Block) {
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

TEST(GymboWorkflowTest, MontyHall) {
    std::string code_str = R"(
    if (car_door == choice) {
        if (door_switch == 1) {
            result = 0;
        } else {
            result = 1;
        }
        return result;
    }

    if (choice != 1 && car_door != 1) {
        host_door = 1;
    } else {
        if (choice != 2 && car_door != 2) {
            host_door = 2;
        } else {
            host_door = 3;
        }
    }

    if (door_switch == 1) {
        if (host_door == 1) {
            if (choice == 2) {
                choice_updated = 3;
            } else {
                choice_updated = 2;
            }
        } else {
            if (host_door == 2) {
                if (choice == 1) {
                    choice_updated = 3;
                } else {
                    choice_updated = 1;
                }
            } else {
                if (choice == 1) {
                    choice_updated = 2;
                } else {
                    choice_updated = 1;
                }
            }
        }
    } else {
        choice_updated = choice;
    }

    if (choice_updated == car_door) {
        result = 1;
    } else {
        result = 0;
    })";

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

    std::unordered_map<int, gymbo::DiscreteDist> var2dist = {
        {0, gymbo::DiscreteUniformDist(1, 3)},
        {1, gymbo::DiscreteUniformDist(1, 3)}};

    std::vector<std::vector<int>> val_candidates;
    for (auto &vd : var2dist) {
        val_candidates.emplace_back(vd.second.vals);
    }
    std::vector<std::vector<int>> D = gymbo::cartesianProduct(val_candidates);

    std::vector<int> doow_switch_candidates = {0, 1};
    std::vector<float> true_expected_val = {1.0f / 3.0f, 2.0f / 3.0f};

    for (int i = 0; i < 2; i++) {
        int door_switch = doow_switch_candidates[i];
        gymbo::SymState init;
        init.set_concrete_val(var_counter["door_switch"], door_switch);

        gymbo::PSExecutor executor(optimizer, maxSAT, maxUNSAT, max_num_trials,
                                   ignore_memory, use_dpll, verbose_level);
        executor.register_random_var(0);
        executor.register_random_var(1);
        executor.run(prg, target_pcs, init, max_depth);

        int num_unique_path_constraints = executor.constraints_cache.size();
        int num_unique_final_states = executor.prob_constraints_table.size();

        std::unordered_map<int, float> params;

        float expected_value = 0.0f;
        for (auto &cc : executor.prob_constraints_table) {
            for (auto &ccv : cc.second) {
                float p = std::get<2>(ccv).eval(params, eps, var2dist, D);
                expected_value +=
                    p *
                    gymbo::wordToFloat(std::get<1>(ccv)[var_counter["result"]]);
            }
        }
        ASSERT_NEAR(expected_value, true_expected_val[i], 1e-6f);
    }
}
