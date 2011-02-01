	.file	"tst.c"
	.text
.globl joe
	.type	joe, @function
joe:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
	.cfi_def_cfa_register 6
	movl	$0, %eax
	call	bob
	leave
	ret
	.cfi_endproc
.LFE0:
	.size	joe, .-joe
.globl heavy
	.type	heavy, @function
heavy:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	movq	%rsp, %rbp
	.cfi_offset 6, -16
	.cfi_def_cfa_register 6
	movl	$0, %eax
	call	joe
	movl	$0, %eax
	call	bob
	leave
	ret
	.cfi_endproc
.LFE1:
	.size	heavy, .-heavy
	.ident	"GCC: (Ubuntu 4.4.1-4ubuntu9) 4.4.1"
	.section	.note.GNU-stack,"",@progbits
