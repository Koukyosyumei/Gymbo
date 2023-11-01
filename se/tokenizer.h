#pragma once
#include <cstdlib>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

inline bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

// Reports an error and exit.
inline void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
inline void error_at(char *user_input, char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_FOR,
  TK_IDENT,
  TK_NUM, // Integer literals
  TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

// Consumes the current token if it matches `op`.
inline bool consume(Token *&token, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

inline bool consume_tok(Token *&token, TokenKind tok) {
  if (token->kind != tok) {
    return false;
  }
  token = token->next;
  return true;
}

inline Token *consume_ident(Token *&token) {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

// Ensure that the current token is `op`.
inline void expect(Token *&token, char *user_input, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    char em[] = "expected \"%s\"";
    error_at(user_input, token->str, em, op);
  }
  token = token->next;
}

// Ensure that the current token is TK_NUM.
inline int expect_number(Token *&token, char *user_input) {
  if (token->kind != TK_NUM) {
    char em[] = "expected a number";
    error_at(user_input, token->str, em);
  }
  int val = token->val;
  token = token->next;
  return val;
}

inline bool at_eof(Token *token) { return token->kind == TK_EOF; }

// Create a new token and add it as the next token of `cur`.
inline Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = (Token *)std::calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

inline bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// Tokenize `user_input` and returns new tokens.
inline Token *tokenize(char *user_input) {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  char LETTER_EQ[] = "==";
  char LETTER_NEQ[] = "!=";
  char LETTER_LEQ[] = "<=";
  char LETTER_GEQ[] = ">=";

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Multi-letter punctuator
    if (startswith(p, LETTER_EQ) || startswith(p, LETTER_NEQ) ||
        startswith(p, LETTER_LEQ) || startswith(p, LETTER_GEQ)) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    // Single-letter punctuator
    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 0);
      cur->len = 1;
      continue;
    }

    char em[] = "invalid token\n";
    error_at(user_input, p, em);
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}