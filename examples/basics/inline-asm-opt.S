	.text
	.file	"inline-asm.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	#APP
	movl	$42, %eax
	#NO_APP
	leaq	.L__unnamed_1(%rip), %rdi
	movl	%eax, %esi
	xorl	%eax, %eax
	jmp	printf@PLT
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"x = %d\n"
	.size	.L__unnamed_1, 8

	.section	".note.GNU-stack","",@progbits
