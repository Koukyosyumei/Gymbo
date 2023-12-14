#include <unordered_map>

#include "../../libgymbo/type.h"
#include "gtest/gtest.h"

TEST(GymboTypeTest, DiscreteUniformDist) {
    gymbo::DiscreteUniformDist udist(1, 4);

    std::vector<int> true_vals = {1, 2, 3, 4};
    std::vector<float> true_probs = {0.25f, 0.25f, 0.25f, 0.25f};

    ASSERT_EQ(udist.vals.size(), 4);
    ASSERT_EQ(udist.probs.size(), 4);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(udist.vals[i], true_vals[i]);
        ASSERT_EQ(udist.probs[i], true_probs[i]);
    }
}

TEST(GymboTypeTest, BernoulliDist) {
    gymbo::BernoulliDist bdist(0.3);

    std::vector<int> true_vals = {0, 1};
    std::vector<float> true_probs = {0.7f, 0.3f};

    ASSERT_EQ(bdist.vals.size(), 2);
    ASSERT_EQ(bdist.probs.size(), 2);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(bdist.vals[i], true_vals[i]);
        ASSERT_EQ(bdist.probs[i], true_probs[i]);
    }
}

TEST(GymboTypeTest, BinomialDist) {
    gymbo::BinomialDist bdist(2, 0.1);
    std::vector<int> true_vals = {0, 1, 2};
    std::vector<float> true_probs = {0.9f * 0.9f, 2.0f * 0.1f * 0.9f, 0.1f * 0.1f};

    ASSERT_EQ(bdist.vals.size(), 3);
    ASSERT_EQ(bdist.probs.size(), 3);

    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(bdist.vals[i], true_vals[i]);
        ASSERT_EQ(bdist.probs[i], true_probs[i]);
    }
}

TEST(GymboTypeTest, SymProbSimple) {
    gymbo::Word32 var_id_0 = 0;
    gymbo::Word32 var_id_1 = 1;

    gymbo::Sym d_cond(
        gymbo::SymType::SEq, new gymbo::Sym(gymbo::SymType::SAny, var_id_0),
        new gymbo::Sym(gymbo::SymType::SCon, gymbo::FloatToWord(1.0)));
    gymbo::Sym n_cond(
        gymbo::SymType::SAnd,
        new gymbo::Sym(
            gymbo::SymType::SEq, new gymbo::Sym(gymbo::SymType::SAny, var_id_1),
            new gymbo::Sym(gymbo::SymType::SCon, gymbo::FloatToWord(2.0))),
        &d_cond);

    gymbo::Sym q_right(gymbo::SymType::SCon, gymbo::FloatToWord(1.0f));

    std::unordered_map<int, gymbo::DiscreteDist> var2dist = {
        {0, gymbo::DiscreteUniformDist(1, 2)},
        {1, gymbo::DiscreteUniformDist(1, 2)}};
    std::vector<std::vector<int>> val_candidates;
    for (auto &vd : var2dist) {
        val_candidates.emplace_back(vd.second.vals);
    }
    std::vector<std::vector<int>> D = gymbo::cartesianProduct(val_candidates);

    gymbo::SymProb prob(&n_cond, &d_cond);
    gymbo::SymType symtype = gymbo::SymType::SLe;
    gymbo::Sym query = prob.query(symtype, q_right, var2dist, D);

    std::string true_q_str =
        "(((((0+(0.250000*[((var_1==2)&&(var_0==1)){0->1,1->1,}]))+(0.250000*[("
        "(var_1==2)&&(var_0==1)){0->2,1->1,}]))+(0.250000*[((var_1==2)&&(var_0="
        "=1)){0->1,1->2,}]))+(0.250000*[((var_1==2)&&(var_0==1)){0->2,1->2,}]))"
        "<=(((((0+(0.250000*[(var_0==1){0->1,1->1,}]))+(0.250000*[(var_0==1){0-"
        ">2,1->1,}]))+(0.250000*[(var_0==1){0->1,1->2,}]))+(0.250000*[(var_0=="
        "1){0->2,1->2,}]))*1))";

    ASSERT_EQ(true_q_str, query.toString(true));

    std::unordered_map<int, float> params = {};
    ASSERT_EQ(0.5, prob.eval(params, 1.0, var2dist, D));
}
