.data
print_fmt_int: .string "%lld\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl test_floats
test_floats:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $4614253070214989087, %rax
 movq %rax, %xmm0
 movl %eax, -12(%rbp)
 movq $4613303445313851803, %rax
 movq %rax, %xmm0
 movq %rax, -24(%rbp)
 movq $4619713684763175813, %rax
 movq %rax, %xmm0
 movq %rax, -32(%rbp)
 movl -12(%rbp), %eax
 movq %rax, %xmm0
 leaq print_fmt_float(%rip), %rdi
 movl $1, %eax
 call printf
 movq -32(%rbp), %rax
 movq %rax, %xmm0
 leaq print_fmt_float(%rip), %rdi
 movl $1, %eax
 call printf
 movl -12(%rbp), %eax
 jmp .end_test_floats
.end_test_floats:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 call test_floats
 movl %eax, -12(%rbp)
 movl -12(%rbp), %eax
 movq %rax, %xmm0
 leaq print_fmt_float(%rip), %rdi
 movl $1, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
