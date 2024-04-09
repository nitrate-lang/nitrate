	.text
	.file	"hello_world.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rax
	leaq	.L_fa37JncCHr(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L_yDsbzayy4c(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L_BWDxS22Jjz(%rip), %rdi
	movl	$1000, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

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
	.asciz	"Long live QUIX! May it reign supreme for %d years to come!\n"
	.size	.L_BWDxS22Jjz, 60

	.section	".note.GNU-stack","",@progbits
