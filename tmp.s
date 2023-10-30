.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  mov rax, rbp
  sub rax, 8
  push rax
  push 2
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  pop rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  push 1
  pop rdi
  pop rax
  cmp rax, rdi
  sete al
  movzb rax, al
  push rax
  pop rax
  cmp rax, 0
  je  .Lend.0
  push 11
  pop rax
  mov rsp, rbp
  pop rbp
  ret
.Lend.0:
  pop rax
  push 17
  pop rax
  mov rsp, rbp
  pop rbp
  ret
  pop rax
  mov rsp, rbp
  pop rbp
  ret
