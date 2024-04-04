	.text
	.file	"hello_me.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	leaq	.L_fa37JncCHr(%rip), %rdi
	leaq	.L_yDsbzayy4c(%rip), %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	%eax, 4(%rsp)
	movl	4(%rsp), %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.type	.L_fa37JncCHr,@object
	.data
.L_fa37JncCHr:
	.asciz	"Hello, %s\n"
	.size	.L_fa37JncCHr, 11

	.type	.L_yDsbzayy4c,@object
.L_yDsbzayy4c:
	.asciz	"Wesley"
	.size	.L_yDsbzayy4c, 7

	.section	".note.GNU-stack","",@progbits
