	.text
	.file	"return-if.q"
	.section	.text.main,"ax",@progbits
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	movb	$1, %al
	xorb	$-1, %al
	testb	$1, %al
	jne	.LBB0_2
	jmp	.LBB0_3
.LBB0_1:                                # %end
	movb	-1(%rsp), %al
	retq
.LBB0_2:
	jmp	.LBB0_1
.LBB0_3:
	movb	$1, %al
	testb	$1, %al
	jne	.LBB0_4
	jmp	.LBB0_5
.LBB0_4:
	movb	$10, -1(%rsp)
	jmp	.LBB0_1
.LBB0_5:
	movb	$1, %al
	xorb	$0, %al
	testb	$1, %al
	jne	.LBB0_6
	jmp	.LBB0_7
.LBB0_6:
	movb	$20, -1(%rsp)
	jmp	.LBB0_1
.LBB0_7:
	movb	$60, -1(%rsp)
	jmp	.LBB0_1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
	.addrsig
