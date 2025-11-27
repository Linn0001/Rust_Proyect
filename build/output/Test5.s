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
 subq $16, %rsp
 movq $0, %rax
 movl %eax, -12(%rbp)
while_0:
 movslq -12(%rbp), %rax
 pushq %rax
 movq $5, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movl $0, %eax
 setl %al
 movzbq %al, %rax
 cmpq $0, %rax
 je endwhile_0
 movslq -12(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -12(%rbp)
 movslq -12(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
 jmp while_0
endwhile_0:
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
