#include <unistd.h>

#include <unordered_map>
#include <utility>

#include "compiler.h"
#include "gd.h"
#include "parser.h"
#include "tokenizer.h"
#include "type.h"

namespace gymbo {

inline std::pair<std::unordered_map<std::string, int>, Prog> gcompile(
    char *user_input) {
    std::unordered_map<std::string, int> var_counter;
    std::vector<gymbo::Node *> code;
    Prog prg;

    Token *token = tokenize(user_input, var_counter);
    generate_ast(token, user_input, code);
    compile_ast(code, prg);

    return std::make_pair(var_counter, prg);
}

inline PathConstraintsTable gexecute(Prog &prg, GDOptimizer &optimizer,
                                    int max_depth, int max_num_trials,
                                    bool ignore_memory, bool use_dpll,
                                    bool verbose_level) {
    SymState init;
    PathConstraintsTable cache_constraints;

    run_gymbo(prg, optimizer, init, cache_constraints, max_depth,
              max_num_trials, ignore_memory, use_dpll, verbose_level);

    return cache_constraints;
}

}  // namespace gymbo