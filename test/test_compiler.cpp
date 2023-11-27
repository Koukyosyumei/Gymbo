#include "../libgymbo/compiler.h"
#include "gtest/gtest.h"

TEST(GymboCompilerTest, Pipeline) {
    char user_input[] = "if (a > 3) return 1;";

    std::unordered_map<std::string, int> vc;
    std::vector<gymbo::Node *> code;
    gymbo::Prog prg;
    gymbo::SymState init;
    gymbo::PathConstraintsTable cache_constraints;

    gymbo::Token *token = gymbo::tokenize(user_input, vc);
    gymbo::generate_ast(token, user_input, code);
    gymbo::compile_ast(code, prg);

    std::vector<gymbo::InstrType> instrstypes = {
        gymbo::InstrType::Push,  gymbo::InstrType::Push,
        gymbo::InstrType::Load,  gymbo::InstrType::Lt,
        gymbo::InstrType::Push,  gymbo::InstrType::Swap,
        gymbo::InstrType::JmpIf, gymbo::InstrType::Nop,
        gymbo::InstrType::Push,  gymbo::InstrType::Jmp,
        gymbo::InstrType::Done,  gymbo::InstrType::Done};

    ASSERT_EQ(prg.size(), instrstypes.size());

    for (int j = 0; j < prg.size(); j++) {
        ASSERT_EQ(prg[j].instr, instrstypes[j]);
    }
}
