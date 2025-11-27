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
 movq $0, %rax
 movq %rax, -16(%rbp)
 movq $1, %rax
 movq %rax, -24(%rbp)
for_0:
 movq -24(%rbp), %rax
 movq %rax, %rcx
 movq $11, %rax
 cmpq %rax, %rcx
 jge endfor_0
 movq -16(%rbp), %rax
 pushq %rax
 movq -24(%rbp), %rax
 pushq %rax
 movq -24(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 imulq %rcx, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -16(%rbp)
 movq -24(%rbp), %rax
 addq $1, %rax
 movq %rax, -24(%rbp)
 jmp for_0
endfor_0:
 movq -16(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
