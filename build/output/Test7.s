.data
print_fmt_int: .string "%lld\n"
print_fmt_uint: .string "%llu\n"
print_fmt_float: .string "%.6f\n"
print_fmt_bool_true: .string "true\n"
print_fmt_bool_false: .string "false\n"

.text
.globl suma_rango
suma_rango:
 pushq %rbp
 movq %rsp, %rbp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 subq $48, %rsp
 movq $0, %rax
 movq %rax, -32(%rbp)
 movq -8(%rbp), %rax
 movq %rax, -40(%rbp)
while_0:
 movq -40(%rbp), %rax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movl $0, %eax
 setle %al
 movzbq %al, %rax
 cmpq $0, %rax
 je endwhile_0
 movq -32(%rbp), %rax
 pushq %rax
 movq -40(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -32(%rbp)
 movq -40(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movq %rax, -40(%rbp)
 jmp while_0
endwhile_0:
 movq -32(%rbp), %rax
 jmp .end_suma_rango
.end_suma_rango:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $1, %rax
 movq %rax, %rdi
 movq $10, %rax
 movq %rax, %rsi
 call suma_rango
 movq %rax, -16(%rbp)
 movq -16(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt_int(%rip), %rdi
 movl $0, %eax
 call printf
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
