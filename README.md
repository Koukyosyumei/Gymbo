# Gymbo: Gradient-based Symbolic Execution 

```
./mysym "if (a < 1) return a;"
```

```
Compiled Stack Machine...

push 3
push 1
load
lt
push 6
swap
jmpIf
nop
push 2
jmp
ret
ret
-------------------------

Start Symbolic Execution...

pc: 0, push 3
Stack: []
Memory: {}
Path Constraints:  1
---
pc: 1, push 1
Stack: [3, ]
Memory: {}
Path Constraints:  1
---
pc: 2, load
Stack: [3, 1, ]
Memory: {}
Path Constraints:  1
---
pc: 3, lt
Stack: [3, var_1, ]
Memory: {}
Path Constraints:  1
---
pc: 4, push 6
Stack: [3 < var_1, ]
Memory: {}
Path Constraints:  1
---
pc: 5, swap
Stack: [3 < var_1, 6, ]
Memory: {}
Path Constraints:  1
---
pc: 6, jmpIf
Stack: [6, 3 < var_1, ]
Memory: {}
Path Constraints:  1
---
pc: 10, ret
Stack: []
Memory: {}
Path Constraints: 3 < var_1 and  1
IS_SAT - 1, params = {1: 4, }
---
pc: 7, nop
Stack: []
Memory: {}
Path Constraints: !(3 < var_1) and  1
IS_SAT - 1, params = {1: 0, }
---
pc: 8, push 2
Stack: []
Memory: {}
Path Constraints: !(3 < var_1) and  1
IS_SAT - 1, params = {1: 0, }
---
pc: 9, jmp
Stack: [2, ]
Memory: {}
Path Constraints: !(3 < var_1) and  1
IS_SAT - 1, params = {1: 0, }
---
pc: 11, ret
Stack: []
Memory: {}
Path Constraints: !(3 < var_1) and  1
IS_SAT - 1, params = {1: 0, }
---
---------------------------
```
