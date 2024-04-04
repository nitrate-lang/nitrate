	.text
	.file	"hello_world.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	leaq	.L_fa37JncCHr(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L_yDsbzayy4c(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L_BWDxS22Jjz(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movb	$0, 7(%rsp)
	movb	7(%rsp), %al
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.type	.L_fa37JncCHr,@object
	.data
	.p2align	4
.L_fa37JncCHr:
	.asciz	"Hi. Behold, the first Hello World program for the QUIX language!\n"
	.size	.L_fa37JncCHr, 66

	.type	.L_yDsbzayy4c,@object
	.p2align	4
.L_yDsbzayy4c:
	.asciz	"This is the first of many programs to come.\n"
	.size	.L_yDsbzayy4c, 45

	.type	.L_BWDxS22Jjz,@object
	.p2align	4
.L_BWDxS22Jjz:
	.asciz	"Long live QUIX!\n"
	.size	.L_BWDxS22Jjz, 17

	.section	".note.GNU-stack","",@progbits
