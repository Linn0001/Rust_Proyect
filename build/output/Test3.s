.data
print_fmt_int: .string "%lld\n"
print_fmt_uint: .string "%llu\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl test_sizes
test_sizes:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $1000000000, %rax
 movw %ax, -10(%rbp)
 movq $1000000000, %rax
 movl %eax, -16(%rbp)
 movq $10000000000, %rax
 movq %rax, -24(%rbp)
 movswq -10(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movslq -16(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movq -24(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 movslq -16(%rbp), %rax
 jmp .end_test_sizes
.end_test_sizes:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 call test_sizes
 movl %eax, -12(%rbp)
 movslq -12(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
