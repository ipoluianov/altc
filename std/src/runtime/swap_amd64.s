# The context switch of tasks (System V AMD64): the callee-saved
# registers, MXCSR and the x87 control word are saved on the stack of the
# task that stops, whose stack pointer is stored in *save; the stack of
# the task that resumes is loaded from load.
#
#   void alt__swap(void **save, void *load)
#
# A new task's stack is prepared by the scheduler so that alt__swap
# "returns" into alt__task_start with the task in %r12.

	.text
	.globl	alt__swap
	.hidden	alt__swap
	.type	alt__swap, @function
alt__swap:
	.cfi_startproc
	pushq	%rbp
	pushq	%rbx
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	subq	$8, %rsp
	stmxcsr	(%rsp)
	fnstcw	4(%rsp)
	movq	%rsp, (%rdi)
	movq	%rsi, %rsp
	ldmxcsr	(%rsp)
	fldcw	4(%rsp)
	addq	$8, %rsp
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbx
	popq	%rbp
	ret
	.cfi_endproc
	.size	alt__swap, .-alt__swap

	.globl	alt__task_start
	.hidden	alt__task_start
	.type	alt__task_start, @function
alt__task_start:
	.cfi_startproc
	.cfi_undefined rip
	xorl	%ebp, %ebp
	movq	%r12, %rdi
	call	alt__task_main
	ud2
	.cfi_endproc
	.size	alt__task_start, .-alt__task_start

	.section .note.GNU-stack,"",@progbits
