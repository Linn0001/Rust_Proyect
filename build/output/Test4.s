.data
print_fmt_int: .string "%lld\n"
print_fmt_uint: .string "%llu\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl suma
suma:
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
 addq %rcx, %rax
 jmp .end_suma
.end_suma:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $1, %rax
 movl %eax, -12(%rbp)
 movq $20, %rax
 movl %eax, -16(%rbp)
 movslq -12(%rbp), %rax
 movq %rax, %rdi
 movslq -16(%rbp), %rax
 movq %rax, %rsi
 call suma
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
