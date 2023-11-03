#pragma once
#include "tokenizer.h"
#include <cstdlib>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

char LETTER_EQ[] = "==";
char LETTER_NEQ[] = "!=";
char LETTER_LQ[] = "<";
char LETTER_LEQ[] = "<=";
char LETTER_GQ[] = ">";
char LETTER_GEQ[] = ">=";
char LETTER_PLUS[] = "+";
char LETTER_MINUS[] = "-";
char LETTER_MUL[] = "*";
char LETTER_DIV[] = "/";
char LETTER_LP[] = "(";
char LETTER_RP[] = ")";
char LETTER_SC[] = ";";
char LETTER_ELSE[] = "else";

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN,
  ND_LVAR,
  ND_NUM, // Integer
  ND_RETURN,
  ND_IF,
  ND_FOR
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  Node *cond;
  Node *then;
  Node *els;
  int val; // Used if kind == ND_NUM
  int offset;
};

Node *assign(Token *&token, char *user_input);
Node *expr(Token *&token, char *user_input);
Node *equality(Token *&token, char *user_input);
Node *relational(Token *&token, char *user_input);
Node *add(Token *&token, char *user_input);
Node *mul(Token *&token, char *user_input);
Node *unary(Token *&token, char *user_input);
Node *primary(Token *&token, char *user_input);
Node *stmt(Token *&token, char *user_input);

Node *new_node(NodeKind kind) {
  Node *node = new Node();
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
/**
 * Parses a relational expression from a C-like language program.
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
 * Parses an assignment statement from a C-like language program.
 *
 * @param token The first token in the assignment statement.
 * @param user_input The source code of the program.
 * @return An AST node representing the assignment statement.
 */
Node *assign(Token *&token, char *user_input) {
  char LETTER_ASS[] = "=";
  Node *node = equality(token, user_input);
  if (consume(token, LETTER_ASS))
    node = new_binary(ND_ASSIGN, node, assign(token, user_input));
  return node;
}

// expr = equality
/**
 * Parses an expression from a C-like language program.
 *
 * @param token The first token in the expression.
 * @param user_input The source code of the program.
 * @return An AST node representing the expression.
 */
Node *expr(Token *&token, char *user_input) {
  return assign(token, user_input);
}

// equality = relational ("==" relational | "!=" relational)*
/**
 * Parses an equality expression from a C-like language program.
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

// add = mul ("+" mul | "-" mul)*
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

// mul = unary ("*" unary | "/" unary)*
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

// unary = ("+" | "-")? unary
//       | primary
inline Node *unary(Token *&token, char *user_input) {
  if (consume(token, LETTER_PLUS))
    return unary(token, user_input);
  if (consume(token, LETTER_MINUS))
    return new_binary(ND_SUB, new_num(0), unary(token, user_input));
  return primary(token, user_input);
}

// primary = "(" expr ")" | num | ident
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
    node->offset = (tok->str[0] - 'a' + 1); //* 8;
    return node;
  }

  return new_num(expect_number(token, user_input));
}

inline Node *stmt(Token *&token, char *user_input) {
  Node *node;

  if (consume_tok(token, TK_RETURN)) {
    node = new Node();
    node->kind = ND_RETURN;
    node->lhs = expr(token, user_input);
    expect(token, user_input, LETTER_SC);
  } else if (consume_tok(token, TK_IF)) {
    node = new Node();
    node->kind = ND_IF;
    expect(token, user_input, LETTER_LP);
    node->cond = expr(token, user_input);
    expect(token, user_input, LETTER_RP);
    node->then = stmt(token, user_input);
    if (consume_tok(token, TK_ELSE)) {
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
 * @return A vector of AST nodes representing the program.
 */
inline void generate_ast(Token *&token, char *user_input,
                         std::vector<Node *> &code) {
  while (!at_eof(token))
    code.emplace_back(stmt(token, user_input));
  code.emplace_back(nullptr);
}
