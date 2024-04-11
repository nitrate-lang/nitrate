	.text
	.file	"inline-asm.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f15do__inline__asm5f1i1p1p,@function
.L_ZJ0f15do__inline__asm5f1i1p1p:
	.cfi_startproc
	#APP
	movl	$42, %eax
	#NO_APP
	movl	%eax, -4(%rsp)
	movl	%eax, -8(%rsp)
	movl	-8(%rsp), %eax
	retq
.Lfunc_end0:
	.size	.L_ZJ0f15do__inline__asm5f1i1p1p, .Lfunc_end0-.L_ZJ0f15do__inline__asm5f1i1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	callq	.L_ZJ0f15do__inline__asm5f1i1p1p
	movl	%eax, 4(%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	movl	%eax, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"x = %d\n"
	.size	.L__unnamed_1, 8

	.section	".note.GNU-stack","",@progbits
