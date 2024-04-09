	.text
	.file	"hello_world.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_3(%rip), %rdi
	movl	$1000, %esi
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

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"Hi. Behold, the first Hello World program for the QUIX language!\n"
	.size	.L__unnamed_1, 66

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"This is the first of many programs to come.\n"
	.size	.L__unnamed_2, 45

	.type	.L__unnamed_3,@object
.L__unnamed_3:
	.asciz	"Long live QUIX! May it reign supreme for %d years to come!\n"
	.size	.L__unnamed_3, 60

	.section	".note.GNU-stack","",@progbits
