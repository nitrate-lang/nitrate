	.text
	.file	"if-statement.q"
	.section	.text.main,"ax",@progbits
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	jmp	.LBB0_2
.LBB0_1:                                # %end
	movb	-1(%rsp), %al
	retq
.LBB0_2:
	jmp	.LBB0_4
# %bb.3:
	movb	$5, %al
	cmpb	$4, %al
	je	.LBB0_6
	jmp	.LBB0_7
.LBB0_4:
	movb	$10, -1(%rsp)
	jmp	.LBB0_1
# %bb.5:
	movb	$20, -1(%rsp)
	jmp	.LBB0_1
.LBB0_6:
	movb	$40, -1(%rsp)
	jmp	.LBB0_1
.LBB0_7:
	movb	$50, -1(%rsp)
	jmp	.LBB0_1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
	.addrsig
