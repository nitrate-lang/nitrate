	.text
	.file	"namespace.q"
	.section	.text.main,"ax",@progbits
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	movb	.L_ZJ0l18Number_CONSTANT__A1c0(%rip), %al
	movb	%al, -2(%rsp)
	movb	.L_ZJ0l18Number_CONSTANT__B1c0(%rip), %al
	movb	%al, -3(%rsp)
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
	.type	.L_ZJ0l18Number_CONSTANT__A1c0,@object # @_ZJ0l18Number_CONSTANT__A1c0
	.section	.data..L_ZJ0l18Number_CONSTANT__A1c0,"aw",@progbits
.L_ZJ0l18Number_CONSTANT__A1c0:
	.byte	10                              # 0xa
	.size	.L_ZJ0l18Number_CONSTANT__A1c0, 1

	.type	.L_ZJ0l18Number_CONSTANT__B1c0,@object # @_ZJ0l18Number_CONSTANT__B1c0
	.section	.data..L_ZJ0l18Number_CONSTANT__B1c0,"aw",@progbits
.L_ZJ0l18Number_CONSTANT__B1c0:
	.byte	20                              # 0x14
	.size	.L_ZJ0l18Number_CONSTANT__B1c0, 1

	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym .L_ZJ0l18Number_CONSTANT__A1c0
	.addrsig_sym .L_ZJ0l18Number_CONSTANT__B1c0
