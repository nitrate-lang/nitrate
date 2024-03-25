	.text
	.file	"fn-stack-alloc.q"
	.section	.text.main,"ax",@progbits
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	movb	$10, -2(%rsp)
	movb	$20, -3(%rsp)
	movb	-2(%rsp), %al
	addb	-3(%rsp), %al
	movb	%al, -1(%rsp)
# %bb.1:                                # %end
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
	.addrsig
