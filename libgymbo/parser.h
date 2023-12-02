/**
 * @file parser.h
 * @brief Implementation of parser
 * @author Hideaki Takahashi
 */

#pragma once
#include <vector>

#include "tokenizer.h"

/**
 * @brief Array representing the equality operator "=="
 */
char LETTER_EQ[] = "==";

/**
 * @brief Array representing the inequality operator "!="
 */
char LETTER_NEQ[] = "!=";

/**
 * @brief Array representing the less than operator "<"
 */
char LETTER_LQ[] = "<";

/**
 * @brief Array representing the less than or equal to operator "<="
 */
char LETTER_LEQ[] = "<=";

/**
 * @brief Array representing the greater than operator ">"
 */
char LETTER_GQ[] = ">";

/**
 * @brief Array representing the greater than or equal to operator ">="
 */
char LETTER_GEQ[] = ">=";

/**
 * @brief Array representing the addition operator "+"
 */
char LETTER_PLUS[] = "+";

/**
 * @brief Array representing the subtraction operator "-"
 */
char LETTER_MINUS[] = "-";

/**
 * @brief Array representing the multiplication operator "*"
 */
char LETTER_MUL[] = "*";

/**
 * @brief Array representing the division operator "/"
 */
char LETTER_DIV[] = "/";

/**
 * @brief Array representing the logical AND operator "&&"
 */
char LETTER_AND[] = "&&";

/**
 * @brief Array representing the logical OR operator "||"
 */
char LETTER_OR[] = "||";

/**
 * @brief Array representing the logical NOT operator "!"
 */
char LETTER_NOT[] = "!";

/**
 * @brief Array representing the left parenthesis "("
 */
char LETTER_LP[] = "(";

/**
 * @brief Array representing the right parenthesis ")"
 */
char LETTER_RP[] = ")";

/**
 * @brief Array representing the semicolon ";"
 */
char LETTER_SC[] = ";";

/**
 * @brief Array representing the keyword "else"
 */
char LETTER_ELSE[] = "else";

/**
 * @brief Array representing the left brace "{"
 */
char LETTER_LB[] = "{";

/**
 * @brief Array representing the right brace "}"
 */
char LETTER_RB[] = "}";

namespace gymbo {

/**
 * @brief Enumeration representing different node kinds in the Abstract Syntax
 * Tree (AST).
 */
typedef enum {
    ND_ADD,  // +
    ND_SUB,  // -
    ND_MUL,  // *
    ND_DIV,  // /
    ND_AND,  // &&
    ND_OR,   // ||
    ND_NOT,  // !
    ND_EQ,   // ==
    ND_NE,   // !=
    ND_LT,   // <
    ND_LE,   // <=
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,  // Integer
    ND_RETURN,
    ND_IF,
    ND_FOR,
    ND_BLOCK,
} NodeKind;

/**
 * @brief Structure representing a node in the Abstract Syntax Tree (AST).
 */
struct Node {
    NodeKind kind;               ///< Node kind
    Node *lhs;                   ///< Left-hand side
    Node *rhs;                   ///< Right-hand side
    Node *cond;                  ///< Condition
    Node *then;                  ///< 'Then' branch
    Node *els;                   ///< 'Else' branch
    std::vector<Node *> blocks;  ///< Vector of child blocks
    float val;                   ///< Used if kind is ND_NUM
    int offset;                  ///< Offset
};

Node *assign(Token *&token, char *user_input);
Node *expr(Token *&token, char *user_input);
Node *equality(Token *&token, char *user_input);
Node *relational(Token *&token, char *user_input);
Node *logical(Token *&token, char *user_input);
Node *add(Token *&token, char *user_input);
Node *mul(Token *&token, char *user_input);
Node *unary(Token *&token, char *user_input);
Node *primary(Token *&token, char *user_input);
Node *stmt(Token *&token, char *user_input);

/**
 * @brief Create a new AST node with the given kind.
 *
 * @param kind The kind of the node.
 * @return A pointer to the newly created node.
 */
Node *new_node(NodeKind kind) {
    Node *node = new Node();
    node->kind = kind;
    return node;
}

/**
 * @brief Create a new binary AST node with the given kind, left-hand side, and
 * right-hand side.
 *
 * @param kind The kind of the node.
 * @param lhs The left-hand side of the binary operation.
 * @param rhs The right-hand side of the binary operation.
 * @return A pointer to the newly created binary node.
 */
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/**
 * @brief Create a new AST node representing a numeric value with the given
 * value.
 *
 * @param val The numeric value of the node.
 * @return A pointer to the newly created numeric node.
 */
Node *new_num(float val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

/**
 * Parses an expression from a C-like language program.
 *
 * expr = assign
 *
 * @param token The first token in the expression.
 * @param user_input The source code of the program.
 * @return An AST node representing the expression.
 */
Node *expr(Token *&token, char *user_input) {
    return assign(token, user_input);
}

/**
 * Parses an assignment statement from a C-like language program.
 *
 * assign = logical ("=" assign)?
 *
 * @param token The first token in the assignment statement.
 * @param user_input The source code of the program.
 * @return An AST node representing the assignment statement.
 */
Node *assign(Token *&token, char *user_input) {
    char LETTER_ASS[] = "=";
    Node *node = logical(token, user_input);
    if (consume(token, LETTER_ASS))
        node = new_binary(ND_ASSIGN, node, assign(token, user_input));
    return node;
}

/**
 * Parses a logical expression from a C-like language program.
 *
 * logical = equality ("&&" equality | "||" equality)*
 *
 * @param token The first token in the logical expression.
 * @param user_input The source code of the program.
 * @return An AST node representing the logical expression.
 */
Node *logical(Token *&token, char *user_input) {
    Node *node = equality(token, user_input);

    for (;;) {
        if (consume(token, LETTER_AND))
            node = new_binary(ND_AND, node, equality(token, user_input));
        else if (consume(token, LETTER_OR))
            node = new_binary(ND_OR, node, equality(token, user_input));
        else
            return node;
    }
}

/**
 * Parses an equality expression from a C-like language program.
 *
 * equality = relational ("==" relational | "!=" relational)*
 *
 * @param token The first token in the equality expression.
 * @param user_input The source code of the program.
 * @return An AST node representing the equality expression.
 */
Node *equality(Token *&token, char *user_input) {
    Node *node = relational(token, user_input);

    for (;;) {
        if (consume(token, LETTER_EQ))
            node = new_binary(ND_EQ, node, relational(token, user_input));
        else if (consume(token, LETTER_NEQ))
            node = new_binary(ND_NE, node, relational(token, user_input));
        else
            return node;
    }
}

/**
 * Parses a relational expression from a C-like language program.
 *
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 *
 * @param token The first token in the relational expression.
 * @param user_input The source code of the program.
 * @return An AST node representing the relational expression.
 */
Node *relational(Token *&token, char *user_input) {
    Node *node = add(token, user_input);

    for (;;) {
        if (consume(token, LETTER_LQ))
            node = new_binary(ND_LT, node, add(token, user_input));
        else if (consume(token, LETTER_LEQ))
            node = new_binary(ND_LE, node, add(token, user_input));
        else if (consume(token, LETTER_GQ))
            node = new_binary(ND_LT, add(token, user_input), node);
        else if (consume(token, LETTER_GEQ))
            node = new_binary(ND_LE, add(token, user_input), node);
        else
            return node;
    }
}

/**
 * @brief Parse and construct an AST node representing addition or subtraction.
 *
 * add = mul ("+" mul | "-" mul)*
 *
 * @param token A reference to the current token.
 * @param user_input The user input string.
 * @return A pointer to the constructed AST node.
 */
inline Node *add(Token *&token, char *user_input) {
    Node *node = mul(token, user_input);

    for (;;) {
        if (consume(token, LETTER_PLUS))
            node = new_binary(ND_ADD, node, mul(token, user_input));
        else if (consume(token, LETTER_MINUS))
            node = new_binary(ND_SUB, node, mul(token, user_input));
        else
            return node;
    }
}

/**
 * @brief Parse and construct an AST node representing multiplication or
 * division.
 *
 * mul = unary ("*" unary | "/" unary)*
 *
 * @param token A reference to the current token.
 * @param user_input The user input string.
 * @return A pointer to the constructed AST node.
 */
inline Node *mul(Token *&token, char *user_input) {
    Node *node = unary(token, user_input);

    for (;;) {
        if (consume(token, LETTER_MUL))
            node = new_binary(ND_MUL, node, unary(token, user_input));
        else if (consume(token, LETTER_DIV))
            node = new_binary(ND_DIV, node, unary(token, user_input));
        else
            return node;
    }
}

/**
 * @brief Parse and construct an AST node representing unary operations.
 *
 * unary = ("+" | "-")? unary
 *       | primary
 *
 * @param token A reference to the current token.
 * @param user_input The user input string.
 * @return A pointer to the constructed AST node.
 */
inline Node *unary(Token *&token, char *user_input) {
    if (consume(token, LETTER_PLUS)) return unary(token, user_input);
    if (consume(token, LETTER_MINUS))
        return new_binary(ND_SUB, new_num(0), unary(token, user_input));
    return primary(token, user_input);
}

/**
 * @brief Parse and construct an AST node representing primary expressions.
 *
 * primary = "(" expr ")" | num | ident
 *
 * @param token A reference to the current token.
 * @param user_input The user input string.
 * @return A pointer to the constructed AST node.
 */
inline Node *primary(Token *&token, char *user_input) {
    if (consume(token, LETTER_LP)) {
        Node *node = expr(token, user_input);
        expect(token, user_input, LETTER_RP);
        return node;
    }

    Token *tok = consume_ident(token);
    if (tok) {
        Node *node = (Node *)std::calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = tok->var_id;
        return node;
    }

    return new_num(expect_number(token, user_input));
}

/**
 * @brief Parse and construct an AST node representing a statement.
 *
 * @param token A reference to the current token.
 * @param user_input The user input string.
 * @return A pointer to the constructed AST node.
 */
inline Node *stmt(Token *&token, char *user_input) {
    Node *node;

    if (consume(token, LETTER_LB)) {
        node = new Node();
        node->kind = ND_BLOCK;
        while (true) {
            node->blocks.emplace_back(stmt(token, user_input));
            if (consume(token, LETTER_RB)) {
                break;
            }
        }
    } else if (consume_tok(token, TOKEN_RETURN)) {
        node = new Node();
        node->kind = ND_RETURN;
        node->lhs = expr(token, user_input);
        expect(token, user_input, LETTER_SC);
    } else if (consume_tok(token, TOKEN_IF)) {
        node = new Node();
        node->kind = ND_IF;
        expect(token, user_input, LETTER_LP);
        node->cond = expr(token, user_input);
        expect(token, user_input, LETTER_RP);
        node->then = stmt(token, user_input);
        if (consume_tok(token, TOKEN_ELSE)) {
            node->els = stmt(token, user_input);
        }
    } else {
        node = expr(token, user_input);
        expect(token, user_input, LETTER_SC);
    }

    return node;
}

/**
 * Parses a C-like language program into an AST.
 *
 * @param token The first token in the program.
 * @param user_input The source code of the program.
 * @param code The vector to store Ast nodes
 */
inline void generate_ast(Token *&token, char *user_input,
                         std::vector<Node *> &code) {
    while (!at_eof(token)) code.emplace_back(stmt(token, user_input));
    code.emplace_back(nullptr);
}
}  // namespace gymbo
