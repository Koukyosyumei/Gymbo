/**
 * @file pipeline.h
 * @brief Basic workloads of gradient-based symbolic execution
 * @author Hideaki Takahashi
 */

#pragma once
#include "compiler.h"

namespace gymbo {

/**
 * @brief Compiles user input into a program, returning variable counts and the
 * compiled program.
 *
 * This function takes a user-provided input in the form of a character array
 * and performs the following steps:
 * 1. Tokenizes the input, counting occurrences of variables using var_counter.
 * 2. Generates an Abstract Syntax Tree (AST) from the tokenized input.
 * 3. Compiles the AST into a program using gymbo::Node objects.
 *
 * @param user_input A character array representing the user-provided input.
 *
 * @return A std::pair containing:
 *   - First element: An std::unordered_map<std::string, int> representing
 * variable counts.
 *   - Second element: A Prog object representing the compiled program.
 *
 * @see tokenize() Function used for tokenization.
 * @see generate_ast() Function used for AST generation.
 * @see compile_ast() Function used for compiling the AST into a program.
 */
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

}  // namespace gymbo
