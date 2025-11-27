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
 subq $64, %rsp
 movq $10, %rax
 movw %ax, -10(%rbp)
 movq $20, %rax
 movq %rax, -24(%rbp)
 movq $5, %rax
 movl %eax, -28(%rbp)
 movswq -10(%rbp), %rax
 pushq %rax
 movq -24(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -40(%rbp)
 movq -24(%rbp), %rax
 pushq %rax
 movl -28(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -48(%rbp)
 movl -28(%rbp), %eax
 pushq %rax
 movq $10, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -56(%rbp)
 movq -40(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movq -48(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movq -56(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_uint(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
