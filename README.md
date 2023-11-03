# Gymbo: Gradient-based Symbolic Execution 

<img src="img/gymbo.drawio.svg">

## Install

```bash
git clone https://github.com/Koukyosyumei/Gymbo.git
./build.sh
```

## CLI Tool

```bash
 ./gymbo "if (a < 3) return 1;"
```

## Header-Only Library

```cpp
char user_input[] = "if (a < 3) return 1;"

std::vector<Node *> code;
Prog prg;
GDOptimizer optimizer(num_itrs, step_size);
SymState init;
PathConstraintsTable cache_constraints;

Token *token = tokenize(user_input);
generate_ast(token, user_input, code);
compile_ast(code, prg);

symRun(prg, optimizer, init, cache_constraints);
```
