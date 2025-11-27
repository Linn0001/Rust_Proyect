.data
print_fmt_int: .string "%lld\n"
print_fmt_uint: .string "%llu\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $10, %rax
 movl %eax, -12(%rbp)
 movq $20, %rax
 movl %eax, -16(%rbp)
 movl -12(%rbp), %eax
 pushq %rax
 movl -16(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -20(%rbp)
 movl -20(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt_uint(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
