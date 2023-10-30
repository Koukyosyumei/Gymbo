#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

//
// Tokenizer
//

typedef enum {
    TK_RESERVED,  // Keywords or punctuators
    TK_RETURN,
    TK_IF,
    TK_FOR,
    TK_IDENT,
    TK_NUM,  // Integer literals
    TK_EOF,  // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
    TokenKind kind;  // Token kind
    Token *next;     // Next token
    int val;         // If kind is TK_NUM, its value
    char *str;       // Token string
    int len;         // Token length
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
    LVar *next;  // 次の変数かNULL
    char *name;  // 変数の名前
    int len;     // 名前の長さ
    int offset;  // RBPからのオフセット
};

// ローカル変数
LVar *locals;

// Input program
char *user_input;

// Current token
Token *token;

int jmp_counter = 0;

// Reports an error and exit.
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");  // print pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

// Consumes the current token if it matches `op`.
bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

bool consume_tok(TokenKind tok) {
    if (token->kind != tok) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT) return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

// Ensure that the current token is `op`.
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected \"%s\"", op);
    token = token->next;
}

// Ensure that the current token is TK_NUM.
int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

// Tokenize `user_input` and returns new tokens.
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // Skip whitespace characters.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Multi-letter punctuator
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
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

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

//
// Parser
//

typedef enum {
    ND_ADD,  // +
    ND_SUB,  // -
    ND_MUL,  // *
    ND_DIV,  // /
    ND_EQ,   // ==
    ND_NE,   // !=
    ND_LT,   // <
    ND_LE,   // <=
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,  // Integer
    ND_RETURN,
    ND_IF,
    ND_FOR
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
    NodeKind kind;  // Node kind
    Node *lhs;      // Left-hand side
    Node *rhs;      // Right-hand side
    Node *cond;
    int val;  // Used if kind == ND_NUM
    int offset;
};

Node *code[100];

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
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

Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *stmt();

Node *code[100];

Node *assign() {
    Node *node = equality();
    if (consume("=")) node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

// expr = equality
Node *expr() { return assign(); }

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
    if (consume("+")) return unary();
    if (consume("-")) return new_binary(ND_SUB, new_num(0), unary());
    return primary();
}

// primary = "(" expr ")" | num | ident
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        /*
        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        */
        return node;
    }
    // if (consume(""))

    return new_num(expect_number());
}

Node *stmt() {
    Node *node;

    if (consume_tok(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    } else if (consume_tok(TK_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->lhs = stmt();
    } else {
        node = expr();
        expect(";");
    }

    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) code[i++] = stmt();
    code[i] = NULL;
}

//
// Code generator
//
//
void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) error("lvar is not a variable");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    if (node->kind == ND_RETURN) {
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }

    switch (node->kind) {
        case ND_IF:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend.%d\n", jmp_counter);
            gen(node->lhs);
            printf(".Lend.%d:\n", jmp_counter);
            jmp_counter++;
            return;
    }

    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) error("%s: invalid number of arguments", argv[0]);

    // Tokenize and parse.
    user_input = argv[1];
    token = tokenize();
    program();

    // Print out the first half of assembly.
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
