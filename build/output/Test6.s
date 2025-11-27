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
 movq $10, %rax
 movl %eax, -12(%rbp)
 movq $1, %rax
 movb %al, -13(%rbp)
.end_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
