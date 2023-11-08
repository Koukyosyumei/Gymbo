#pragma once
#include <cstdlib>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace gymbo {
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
  TOKEN_RESERVED, // Keywords or punctuators
  TOKEN_RETURN,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_FOR,
  TOKEN_IDENT,
  TOKEN_NUM, // Integer literals
  TOKEN_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TOKEN_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

/**
 * Consumes the current token if it matches `op`.
 *
 * @param token A pointer to the current token.
 * @param op The token to consume.
 * @return True if the token was consumed, false otherwise.
 */
inline bool consume(Token *&token, char *op) {
  if (token->kind != TOKEN_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

/**
 * Consumes the current token if it matches `op`.
 *
 * @param token A pointer to the current token.
 * @param op The token to consume.
 * @return True if the token was consumed, false otherwise.
 */
inline bool consume_tok(Token *&token, TokenKind tok) {
  if (token->kind != tok) {
    return false;
  }
  token = token->next;
  return true;
}

/**
 * Consumes the current token if it is an identifier.
 *
 * @param token A pointer to the current token.
 * @return A pointer to the consumed identifier token, or NULL if the current
 * token is not an identifier.
 */
inline Token *consume_ident(Token *&token) {
  if (token->kind != TOKEN_IDENT)
    return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

/**
 * Ensures that the current token matches `op`.
 *
 * @param token A pointer to the current token.
 * @param user_input The source code of the program.
 * @param op The token to expect.
 * @throws An error if the current token does not match `op`.
 */
inline void expect(Token *&token, char *user_input, char *op) {
  if (token->kind != TOKEN_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    char em[] = "expected \"%s\"";
    error_at(user_input, token->str, em, op);
  }
  token = token->next;
}

/**
 * Ensures that the current token is a number.
 *
 * @param token A pointer to the current token.
 * @param user_input The source code of the program.
 * @throws An error if the current token is not a number.
 */
inline int expect_number(Token *&token, char *user_input) {
  if (token->kind != TOKEN_NUM) {
    char em[] = "expected a number";
    error_at(user_input, token->str, em);
  }
  int val = token->val;
  token = token->next;
  return val;
}

/**
 * Checks if the current token is at the end of the program.
 *
 * @param token A pointer to the current token.
 * @return True if the current token is at the end of the program, false
 * otherwise.
 */
inline bool at_eof(Token *token) { return token->kind == TOKEN_EOF; }

/**
 * Creates a new token and adds it as the next token of `cur`.
 *
 * @param kind The kind of token to create.
 * @param cur A pointer to the current token.
 * @param str The string of the token to create.
 * @param len The length of the token to create.
 * @return A pointer to the newly created token.
 */
inline Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = (Token *)std::calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

/**
 * Checks if the string `p` starts with the string `q`.
 *
 * @param p A pointer to the string to check.
 * @param q A pointer to the string to check against.
 * @return True if `p` starts with `q`, false otherwise.
 */
inline bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

/**
 * Tokenizes a given string and returns a linked list of tokens.
 *
 * @param user_input The string to be tokenized.
 * @return A linked list of tokens.
 */
inline Token *tokenize(char *user_input) {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  char LETTER_EQ[] = "==";
  char LETTER_NEQ[] = "!=";
  char LETTER_LEQ[] = "<=";
  char LETTER_GEQ[] = ">=";
  char LETTER_AND[] = "&&";
  char LETTER_OR[] = "||";

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Multi-letter punctuator
    if (startswith(p, LETTER_EQ) || startswith(p, LETTER_NEQ) ||
        startswith(p, LETTER_LEQ) || startswith(p, LETTER_GEQ) ||
        startswith(p, LETTER_AND) || startswith(p, LETTER_OR)) {
      cur = new_token(TOKEN_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TOKEN_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TOKEN_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TOKEN_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    // Single-letter punctuator
    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TOKEN_RESERVED, cur, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TOKEN_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TOKEN_IDENT, cur, p++, 0);
      cur->len = 1;
      continue;
    }

    char em[] = "invalid token\n";
    error_at(user_input, p, em);
  }

  new_token(TOKEN_EOF, cur, p, 0);
  return head.next;
}
} // namespace gymbo
