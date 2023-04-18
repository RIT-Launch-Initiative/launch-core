	.file	"test2.cpp"
	.text
	.section	.rodata
	.align 8
	.type	_ZL13MAX_NUM_TASKS, @object
	.size	_ZL13MAX_NUM_TASKS, 8
_ZL13MAX_NUM_TASKS:
	.quad	64
	.align 8
	.type	_ZL14MAX_CALL_DEPTH, @object
	.size	_ZL14MAX_CALL_DEPTH, 8
_ZL14MAX_CALL_DEPTH:
	.quad	64
.LC0:
	.string	"task0 - %i\n"
	.text
	.globl	_Z5task0Pv
	.type	_Z5task0Pv, @function
_Z5task0Pv:
.LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	sched_dispatched(%rip), %ecx
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	520+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rdx
	leaq	1(%rdx), %rsi
	movslq	%ecx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	520+sched_jump(%rip), %rax
	movq	%rsi, (%rcx,%rax)
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rcx,%rax), %rax
	cmpq	%rax, %rdx
	setb	%al
	testb	%al, %al
	je	.L2
	movl	sched_dispatched(%rip), %ecx
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	addq	%rax, %rax
	nop
.L6:
	movl	sched_dispatched(%rip), %edx
	movslq	%edx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rcx,%rax), %rax
	leaq	-1(%rax), %rcx
	movslq	%edx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	%rcx, (%rdx,%rax)
	jmp	.L7
.L2:
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	520+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rax
	movl	%eax, -4(%rbp)
	movl	$0, -8(%rbp)
.L7:
	movl	-8(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	addl	$1, -8(%rbp)
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rax
	cmpq	$64, %rax
	sete	%al
	movzbl	%al, %eax
	testq	%rax, %rax
	je	.L4
	movl	$1, %eax
	jmp	.L5
.L4:
	movl	sched_dispatched(%rip), %edi
	movl	sched_dispatched(%rip), %ecx
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rdx
	leaq	1(%rdx), %rsi
	movslq	%ecx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	%rsi, (%rcx,%rax)
	movslq	%edi, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	leaq	0(,%rax,8), %rcx
	leaq	sched_jump(%rip), %rax
	leaq	.L6(%rip), %rdx
	movq	%rdx, (%rcx,%rax)
	movl	$2, %eax
.L5:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	_Z5task0Pv, .-_Z5task0Pv
	.section	.rodata
.LC1:
	.string	"task1 - %i\n"
	.text
	.globl	_Z5task1Pv
	.type	_Z5task1Pv, @function
_Z5task1Pv:
.LFB16:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	sched_dispatched(%rip), %ecx
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	520+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rdx
	leaq	1(%rdx), %rsi
	movslq	%ecx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	520+sched_jump(%rip), %rax
	movq	%rsi, (%rcx,%rax)
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rcx,%rax), %rax
	cmpq	%rax, %rdx
	setb	%al
	testb	%al, %al
	je	.L9
	movl	sched_dispatched(%rip), %ecx
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	addq	%rax, %rax
	nop
.L13:
	movl	sched_dispatched(%rip), %edx
	movslq	%edx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rcx,%rax), %rax
	leaq	-1(%rax), %rcx
	movslq	%edx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	%rcx, (%rdx,%rax)
	jmp	.L14
.L9:
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	520+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rax
	movl	%eax, -4(%rbp)
	movl	$0, -8(%rbp)
.L14:
	movl	-8(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC1(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	addl	$1, -8(%rbp)
	movl	sched_dispatched(%rip), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rax
	cmpq	$64, %rax
	sete	%al
	movzbl	%al, %eax
	testq	%rax, %rax
	je	.L11
	movl	$1, %eax
	jmp	.L12
.L11:
	movl	sched_dispatched(%rip), %edi
	movl	sched_dispatched(%rip), %ecx
	movslq	%ecx, %rdx
	movq	%rdx, %rax
	salq	$5, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	512+sched_jump(%rip), %rax
	movq	(%rdx,%rax), %rdx
	leaq	1(%rdx), %rsi
	movslq	%ecx, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	512+sched_jump(%rip), %rax
	movq	%rsi, (%rcx,%rax)
	movslq	%edi, %rcx
	movq	%rcx, %rax
	salq	$5, %rax
	addq	%rcx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	leaq	0(,%rax,8), %rcx
	leaq	sched_jump(%rip), %rax
	leaq	.L13(%rip), %rdx
	movq	%rdx, (%rcx,%rax)
	movl	$2, %eax
.L12:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	_Z5task1Pv, .-_Z5task1Pv
	.globl	stack0
	.bss
	.align 32
	.type	stack0, @object
	.size	stack0, 4096
stack0:
	.zero	4096
	.globl	stack1
	.align 32
	.type	stack1, @object
	.size	stack1, 4096
stack1:
	.zero	4096
	.text
	.globl	main
	.type	main, @function
main:
.LFB17:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	leaq	stack0(%rip), %rax
	addq	$4096, %rax
	movq	%rax, -16(%rbp)
	leaq	stack1(%rip), %rax
	addq	$4096, %rax
	movq	%rax, -8(%rbp)
.L16:
	movq	-16(%rbp), %rax
#APP
# 41 "test2.cpp" 1
		 mov %rax,%rsp
# 0 "" 2
#NO_APP
	movl	$0, %edi
	call	_Z5task0Pv
	movq	-8(%rbp), %rax
#APP
# 48 "test2.cpp" 1
		 mov %rax,%rsp
# 0 "" 2
#NO_APP
	movl	$0, %edi
	call	_Z5task1Pv
	jmp	.L16
	.cfi_endproc
.LFE17:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
