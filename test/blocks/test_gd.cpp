#include "../../libgymbo/gd.h"
#include "gtest/gtest.h"

int num_itrs = 100;
float step_size = 1.0f;
float eps = 1.0f;
int param_low = -10;
int param_high = 10;
int seed = 42;
bool sign_grad = true;
bool ignore_memory = false;
bool init_param_uniform_int = true;

void initialize_params(std::unordered_map<int, float> &params,
                       gymbo::SymState &state, bool ignore_memory) {
    params = {};
    if (!ignore_memory) {
        for (auto &p : state.mem) {
            params.emplace(std::make_pair(p.first, wordToFloat(p.second)));
        }
    }
}

TEST(GymboGDTest, Count) {
    gymbo::Word32 var_id_0 = 0;
    gymbo::Word32 var_id_1 = 1;

    gymbo::Sym *cond_0 = new gymbo::Sym(
        gymbo::SymType::SCnt,
        new gymbo::Sym(gymbo::SymType::SEq,
                       new gymbo::Sym(gymbo::SymType::SAny, var_id_0),
                       new gymbo::Sym(gymbo::SymType::SCon, FloatToWord(3.0))));

    gymbo::Sym *cond_1 = new gymbo::Sym(
        gymbo::SymType::SCnt,
        new gymbo::Sym(gymbo::SymType::SEq,
                       new gymbo::Sym(gymbo::SymType::SAny, var_id_1),
                       new gymbo::Sym(gymbo::SymType::SCon, FloatToWord(7.0))));

    gymbo::Sym cond_a =
        gymbo::Sym(gymbo::SymType::SEq,
                   new gymbo::Sym(gymbo::SymType::SAdd, cond_0, cond_1),
                   new gymbo::Sym(gymbo::SymType::SCon, FloatToWord(0.0)));

    gymbo::Sym cond_b =
        gymbo::Sym(gymbo::SymType::SEq,
                   new gymbo::Sym(gymbo::SymType::SAdd, cond_0, cond_1),
                   new gymbo::Sym(gymbo::SymType::SCon, FloatToWord(1.0)));

    gymbo::Sym cond_c =
        gymbo::Sym(gymbo::SymType::SEq,
                   new gymbo::Sym(gymbo::SymType::SAdd, cond_0, cond_1),
                   new gymbo::Sym(gymbo::SymType::SCon, FloatToWord(2.0)));

    gymbo::GDOptimizer optimizer(num_itrs, step_size, eps, param_low,
                                 param_high, sign_grad, init_param_uniform_int,
                                 seed);

    bool is_sat = false;
    gymbo::SymState state;
    std::unordered_map<int, float> params = {};

    state.path_constraints = {cond_a};
    initialize_params(params, state, ignore_memory);
    is_sat = optimizer.solve(state.path_constraints, params);
    ASSERT_TRUE(is_sat);
    ASSERT_TRUE(params[0] != 3.0 && params[1] != 7.0);

    state.path_constraints = {cond_b};
    initialize_params(params, state, ignore_memory);
    is_sat = optimizer.solve(state.path_constraints, params);
    ASSERT_TRUE(is_sat);
    ASSERT_TRUE((params[0] == 3.0 || params[1] == 7.0) &&
                (params[0] != 3.0 || params[1] != 7.0));

    state.path_constraints = {cond_c};
    initialize_params(params, state, ignore_memory);
    is_sat = optimizer.solve(state.path_constraints, params);
    ASSERT_TRUE(is_sat);
    ASSERT_TRUE(params[0] == 3.0 && params[1] == 7.0);
}
