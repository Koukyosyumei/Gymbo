#pragma once
#include "tokenizer.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

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
  int val; // Used if kind == ND_NUM
  int offset;
};

inline Node *new_node(NodeKind kind) {
  Node *node = new Node();
  node->kind = kind;
  return node;
}

inline Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

inline Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *stmt();

inline Node *assign(Token *token) {
  char LETTER_ASS[] = "=";
  Node *node = equality();
  if (consume(token, LETTER_ASS))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

// expr = equality
inline Node *expr() { return assign(); }

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

// equality = relational ("==" relational | "!=" relational)*
inline Node *equality(Token *token) {

  Node *node = relational();

  for (;;) {
    if (consume(token, LETTER_EQ))
      node = new_binary(ND_EQ, node, relational());
    else if (consume(token, LETTER_NEQ))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
inline Node *relational(Token *token) {
  Node *node = add();

  for (;;) {
    if (consume(token, LETTER_LQ))
      node = new_binary(ND_LT, node, add());
    else if (consume(token, LETTER_LEQ))
      node = new_binary(ND_LE, node, add());
    else if (consume(token, LETTER_GQ))
      node = new_binary(ND_LT, add(), node);
    else if (consume(token, LETTER_GEQ))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
inline Node *add(Token *token) {
  Node *node = mul();

  for (;;) {
    if (consume(token, LETTER_PLUS))
      node = new_binary(ND_ADD, node, mul());
    else if (consume(token, LETTER_MINUS))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
inline Node *mul(Token *token) {
  Node *node = unary();

  for (;;) {
    if (consume(token, LETTER_MUL))
      node = new_binary(ND_MUL, node, unary());
    else if (consume(token, LETTER_DIV))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
inline Node *unary(Token *token) {
  if (consume(token, LETTER_PLUS))
    return unary();
  if (consume(token, LETTER_MINUS))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num | ident
inline Node *primary(Token *token, char *user_input) {
  if (consume(token, LETTER_LP)) {
    Node *node = expr();
    expect(token, user_input, LETTER_RP);
    return node;
  }

  Token *tok = consume_ident(token);
  if (tok) {
    Node *node = new Node();
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_num(expect_number(token, user_input));
}

inline Node *stmt(Token *token, char *user_input) {
  Node *node;

  if (consume_tok(token, TK_RETURN)) {
    node = new Node();
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(token, user_input, LETTER_SC);
  } else if (consume_tok(token, TK_IF)) {
    node = new Node();
    node->kind = ND_IF;
    expect(token, user_input, LETTER_LP);
    node->cond = expr();
    expect(token, user_input, LETTER_RP);
    node->lhs = stmt();
  } else {
    node = expr();
    expect(token, user_input, LETTER_SC);
  }

  return node;
}

inline void program(Token *token, char *user_input, std::vector<Node *> code) {
  while (!at_eof(token))
    code.emplace_back(stmt(token, user_input));
  code.emplace_back(NULL);
}
