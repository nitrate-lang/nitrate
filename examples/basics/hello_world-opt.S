	.text
	.file	"hello_world.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rax
	leaq	.Lstr(%rip), %rdi
	callq	puts@PLT
	leaq	.Lstr.1(%rip), %rdi
	callq	puts@PLT
	leaq	.L__unnamed_1(%rip), %rdi
	movl	$1000, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"Long live QUIX! May it reign supreme for %d years to come!\n"
	.size	.L__unnamed_1, 60

	.type	.Lstr,@object
.Lstr:
	.asciz	"Hi. Behold, the first Hello World program for the QUIX language!"
	.size	.Lstr, 65

	.type	.Lstr.1,@object
.Lstr.1:
	.asciz	"This is the first of many programs to come."
	.size	.Lstr.1, 44

	.section	".note.GNU-stack","",@progbits
