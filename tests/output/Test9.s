.data
print_fmt_int: .string "%lld\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl process
process:
 pushq %rbp
 movq %rsp, %rbp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movq %rdx, -24(%rbp)
 subq $48, %rsp
 movq -24(%rbp), %rax
 movq %rax, -40(%rbp)
 movq -40(%rbp), %rax
 pushq %rax
 movslq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -40(%rbp)
 movq -40(%rbp), %rax
 jmp .end_process
.end_process:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $5, %rax
 movq %rax, %rdi
 movq $100, %rax
 movq %rax, %rsi
 movq $1000, %rax
 movq %rax, %rdx
 call process
 movq %rax, -16(%rbp)
 movq $100, %rax
 movl %eax, -20(%rbp)
 movq -16(%rbp), %rax
 pushq %rax
 movslq -20(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movq $0, %rax
 jmp .end_main
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
