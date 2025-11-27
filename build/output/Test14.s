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
 movabsq $4612811918334230528, %rax
 movq %rax, %xmm0
 movq %rax, -16(%rbp)
 movabsq $4616189618054758400, %rax
 movq %rax, %xmm0
 movq %rax, -24(%rbp)
 movq -16(%rbp), %rax
 pushq %rax
 movq -24(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -32(%rbp)
 movq -32(%rbp), %rax
 movq %rax, %xmm0
 leaq print_fmt_float(%rip), %rdi
 movl $1, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
