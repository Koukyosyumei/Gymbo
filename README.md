# Gymbo: Gradient-based Symbolic Execution 

<img src="img/gymbo.drawio.svg">

Gymbo is a Proof of Concept for a Gradient-based Symbolic Execution Engine, implemented from scratch. Building on recent advancements that utilize gradient descent to solve SMT-like formulas [3, 4], Gymbo leverages gradient descent to discover input values that fulfill each path constraint during symbolic execution.

Gymbo is entirely implemented in C++ and requires only standard libraries. The process of compiling from source code to stack machines is based on the implementation of rui314/chibicc [1], while the symbolic execution approach is inspired by beala/symbolic [2].

Compared to other prominent symbolic execution tools, Gymbo's implementation is notably simpler and more compact. We hope that this project will assist individuals in grasping the fundamental principles of symbolic execution and SMT problem-solving through gradient descent.

Please note that Gymbo is currently under development and may have bugs. Your feedback and contributions are always greatly appreciated.

## Install

```bash
git clone https://github.com/Koukyosyumei/Gymbo.git
./build.sh
```

## Input Source Code Grammar

Gymbo presently supports C-like programs with the following BNF grammar:

```
program    = stmt*
stmt       = expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)? 
           | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
```

> Please note that Gymbo only tracks + and - operations within conditional statements for path constraints.

## CLI Tool

```bash
./gymbo "if (a < 3) if (a > 4) return 1;" -v 0

>Compiling the input program...
>Start Symbolic Execution...
>---------------------------
>Result Summary
>#Total Path Constraints: 4
>#SAT: 3
>#UNSAT: 1
>List of UNSAT Path Constraints
># var_1 < 3 and 4 < var_1 and  1
```

The tool accepts the following command-line options:

- `-d`: Set the maximum depth for symbolic execution (default: 256).
- `-v`: Set the verbosity level (default: 1). Use -1 for minimal output.
        -1: the number of satisfiable path constraints and unsatisfiable path constraints.
         0: + the list of unsatisfiable path constraints.
         1: + trace at each operation, including the content of the virtual stack and memory.
         2: + complete stack machine.
- `-i`: Set the number of iterations for gradient descent (default: 100).
- `-a`: Set the step size for gradient descent (default: 1).

## Header-Only Library

```cpp
#include "libgymbo/compiler.h"
#include "libgymbo/gd.h"
#include "libgymbo/parser.h"
#include "libgymbo/tokenizer.h"
#include "libgymbo/type.h"

char user_input[] = "if (a < 3) return 1;"

std::vector<Node *> code;
Prog prg;
GDOptimizer optimizer(num_itrs, step_size);
SymState init;
PathConstraintsTable cache_constraints;

Token *token = tokenize(user_input);
generate_ast(token, user_input, code);
compile_ast(code, prg);

run_gymbo(prg, optimizer, init, cache_constraints);
```

## Reference

- [1] https://github.com/rui314/chibicc
- [2] https://github.com/beala/symbolic
- [3] Chen, Peng, Jianzhong Liu, and Hao Chen. "Matryoshka: fuzzing deeply nested branches." Proceedings of the 2019 ACM SIGSAC Conference on Computer and Communications Security. 2019.
- [4] Minghao Liu, Kunhang Lv, Pei Huang, Rui Han, Fuqi Jia, Yu Zhang, Feifei Ma, Jian Zhang. "NRAgo: Solving SMT(NRA) Formulas with Gradient-based Optimization." IEEE/ACM International Conference on Automated Software Engineering, 2023
