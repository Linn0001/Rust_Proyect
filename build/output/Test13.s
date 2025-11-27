.data
print_fmt_int: .string "%lld\n"
print_fmt_uint: .string "%llu\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl max
max:
 pushq %rbp
 movq %rsp, %rbp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 subq $32, %rsp
 movslq -8(%rbp), %rax
 pushq %rax
 movslq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movl $0, %eax
 setg %al
 movzbq %al, %rax
 cmpq $0, %rax
 je tern_else_0
 movslq -8(%rbp), %rax
 jmp tern_end_0
tern_else_0:
 movslq -16(%rbp), %rax
tern_end_0:
 jmp .end_max
.end_max:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $15, %rax
 movl %eax, -12(%rbp)
 movq $9, %rax
 movl %eax, -16(%rbp)
 movslq -12(%rbp), %rax
 movq %rax, %rdi
 movslq -16(%rbp), %rax
 movq %rax, %rsi
 call max
 movl %eax, -20(%rbp)
 movslq -20(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
