	.file	"sched.cpp"
	.text
	.section	.rodata
	.align 8
	.type	_ZL13MAX_NUM_TASKS, @object
	.size	_ZL13MAX_NUM_TASKS, 8
_ZL13MAX_NUM_TASKS:
	.quad	64
	.local	_ZL5tasks
	.comm	_ZL5tasks,3072,32
	.local	_ZL7ready_q
	.comm	_ZL7ready_q,4224,32
	.text
	.globl	_Z10sleep_sortRP6task_sS1_
	.type	_Z10sleep_sortRP6task_sS1_, @function
_Z10sleep_sortRP6task_sS1_:
.LFB45:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movl	28(%rax), %edx
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movl	28(%rax), %eax
	cmpl	%eax, %edx
	jbe	.L2
	movl	$1, %eax
	jmp	.L3
.L2:
	movl	$0, %eax
.L3:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE45:
	.size	_Z10sleep_sortRP6task_sS1_, .-_Z10sleep_sortRP6task_sS1_
	.local	_ZL7sleep_q
	.comm	_ZL7sleep_q,4232,32
	.globl	_Z10dummy_timev
	.type	_Z10dummy_timev, @function
_Z10dummy_timev:
.LFB46:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE46:
	.size	_Z10dummy_timev, .-_Z10dummy_timev
	.section	.data.rel.local,"aw",@progbits
	.align 8
	.type	_ZL8get_time, @object
	.size	_ZL8get_time, 8
_ZL8get_time:
	.quad	_Z10dummy_timev
	.text
	.globl	_Z10sched_initPFjvE
	.type	_Z10sched_initPFjvE, @function
_Z10sched_initPFjvE:
.LFB47:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L7
	movl	$0, %eax
	jmp	.L8
.L7:
	movq	-8(%rbp), %rax
	movq	%rax, _ZL8get_time(%rip)
	movl	$1, %eax
.L8:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE47:
	.size	_Z10sched_initPFjvE, .-_Z10sched_initPFjvE
	.globl	_Z11sched_startPF7RetTypePvES0_
	.type	_Z11sched_startPF7RetTypePvES0_, @function
_Z11sched_startPF7RetTypePvES0_:
.LFB49:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, -4(%rbp)
.L13:
	movl	-4(%rbp), %eax
	cltq
	cmpq	$63, %rax
	ja	.L10
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	_ZL5tasks(%rip), %rax
	movl	(%rdx,%rax), %eax
	testl	%eax, %eax
	jne	.L11
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	_ZL5tasks(%rip), %rax
	movl	$1, (%rdx,%rax)
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+_ZL5tasks(%rip), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, (%rcx,%rax)
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	16+_ZL5tasks(%rip), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, (%rcx,%rax)
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	24+_ZL5tasks(%rip), %rax
	movl	-4(%rbp), %edx
	movl	%edx, (%rcx,%rax)
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	40+_ZL5tasks(%rip), %rax
	movq	$0, (%rdx,%rax)
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	leaq	_ZL5tasks(%rip), %rdx
	addq	%rdx, %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	movq	%rax, %rcx
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	32+_ZL5tasks(%rip), %rax
	movq	%rcx, (%rdx,%rax)
	movl	-4(%rbp), %eax
	jmp	.L12
.L11:
	addl	$1, -4(%rbp)
	jmp	.L13
.L10:
	movl	$-1, %eax
.L12:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE49:
	.size	_Z11sched_startPF7RetTypePvES0_, .-_Z11sched_startPF7RetTypePvES0_
	.type	_ZL19_sched_wakeup_tasksv, @function
_ZL19_sched_wakeup_tasksv:
.LFB50:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
.L19:
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv
	movq	%rax, -16(%rbp)
	cmpq	$0, -16(%rbp)
	je	.L20
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	_ZL8get_time(%rip), %rax
	call	*%rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	28(%rax), %eax
	cmpl	%eax, %edx
	setnb	%al
	testb	%al, %al
	je	.L21
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv
	movq	-8(%rbp), %rax
	movl	$1, (%rax)
	movq	-8(%rbp), %rax
	movq	$0, 40(%rax)
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 32(%rax)
	jmp	.L19
.L20:
	nop
	jmp	.L14
.L21:
	nop
.L14:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE50:
	.size	_ZL19_sched_wakeup_tasksv, .-_ZL19_sched_wakeup_tasksv
	.globl	_Z12sched_selectv
	.type	_Z12sched_selectv, @function
_Z12sched_selectv:
.LFB51:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	call	_ZL19_sched_wakeup_tasksv
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE4peekEv
	movq	%rax, -16(%rbp)
	cmpq	$0, -16(%rbp)
	jne	.L23
	movl	$0, %eax
	jmp	.L24
.L23:
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE3popEv
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-8(%rbp), %rax
.L24:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE51:
	.size	_Z12sched_selectv, .-_Z12sched_selectv
	.globl	_Z10sched_killi
	.type	_Z10sched_killi, @function
_Z10sched_killi:
.LFB52:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	-20(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	leaq	_ZL5tasks(%rip), %rdx
	addq	%rdx, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	testq	%rax, %rax
	je	.L26
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 40(%rax)
	jmp	.L27
.L26:
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	je	.L27
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 32(%rax)
.L27:
	movq	-8(%rbp), %rax
	movl	$0, (%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE52:
	.size	_Z10sched_killi, .-_Z10sched_killi
	.globl	_Z11sched_sleepij
	.type	_Z11sched_sleepij, @function
_Z11sched_sleepij:
.LFB53:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	-20(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	leaq	_ZL5tasks(%rip), %rdx
	addq	%rdx, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	testq	%rax, %rax
	je	.L29
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	jmp	.L30
.L29:
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	je	.L30
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 32(%rax)
.L30:
	movq	-8(%rbp), %rax
	movl	$2, (%rax)
	movq	_ZL8get_time(%rip), %rax
	call	*%rax
	movl	%eax, %edx
	movl	-24(%rbp), %eax
	addl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 28(%rax)
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 40(%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE53:
	.size	_Z11sched_sleepij, .-_Z11sched_sleepij
	.globl	_Z10sched_wakei
	.type	_Z10sched_wakei, @function
_Z10sched_wakei:
.LFB54:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	-20(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	leaq	_ZL5tasks(%rip), %rdx
	addq	%rdx, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$3, %eax
	je	.L32
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$2, %eax
	jne	.L36
.L32:
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	jne	.L37
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	testq	%rax, %rax
	je	.L35
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 40(%rax)
.L35:
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-8(%rbp), %rax
	movl	$1, (%rax)
	jmp	.L31
.L36:
	nop
	jmp	.L31
.L37:
	nop
.L31:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE54:
	.size	_Z10sched_wakei, .-_Z10sched_wakei
	.globl	_Z11sched_blocki
	.type	_Z11sched_blocki, @function
_Z11sched_blocki:
.LFB55:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	-20(%rbp), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$4, %rax
	leaq	_ZL5tasks(%rip), %rdx
	addq	%rdx, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	je	.L39
	movq	-8(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 32(%rax)
	jmp	.L40
.L39:
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	testq	%rax, %rax
	je	.L40
	movq	-8(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	movq	-8(%rbp), %rax
	movq	$0, 40(%rax)
.L40:
	movq	-8(%rbp), %rax
	movl	$3, (%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE55:
	.size	_Z11sched_blocki, .-_Z11sched_blocki
	.section	.text._ZN5QueueIP6task_sEC2Ev,"axG",@progbits,_ZN5QueueIP6task_sEC5Ev,comdat
	.align 2
	.weak	_ZN5QueueIP6task_sEC2Ev
	.type	_ZN5QueueIP6task_sEC2Ev, @function
_ZN5QueueIP6task_sEC2Ev:
.LFB58:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	leaq	16+_ZTV5QueueIP6task_sE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE58:
	.size	_ZN5QueueIP6task_sEC2Ev, .-_ZN5QueueIP6task_sEC2Ev
	.weak	_ZN5QueueIP6task_sEC1Ev
	.set	_ZN5QueueIP6task_sEC1Ev,_ZN5QueueIP6task_sEC2Ev
	.section	.text._ZN5alloc5QueueIP6task_sLm64EEC2Ev,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EEC5Ev,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EEC2Ev
	.type	_ZN5alloc5QueueIP6task_sLm64EEC2Ev, @function
_ZN5alloc5QueueIP6task_sLm64EEC2Ev:
.LFB60:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sEC2Ev
	movq	-8(%rbp), %rax
	addq	$32, %rax
	movq	%rax, %rdi
	call	_ZN5QueueIP6task_sEC2Ev
	leaq	16+_ZTVN5alloc5QueueIP6task_sLm64EEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	leaq	88+_ZTVN5alloc5QueueIP6task_sLm64EEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-8(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	call	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC1Ev
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE60:
	.size	_ZN5alloc5QueueIP6task_sLm64EEC2Ev, .-_ZN5alloc5QueueIP6task_sLm64EEC2Ev
	.weak	_ZN5alloc5QueueIP6task_sLm64EEC1Ev
	.set	_ZN5alloc5QueueIP6task_sLm64EEC1Ev,_ZN5alloc5QueueIP6task_sLm64EEC2Ev
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EEC5EPFbRS2_S4_E,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E, @function
_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E:
.LFB63:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E
	movq	-8(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	call	_ZN5QueueIP6task_sEC2Ev
	leaq	16+_ZTVN5alloc11SortedQueueIP6task_sLm64EEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	leaq	88+_ZTVN5alloc11SortedQueueIP6task_sLm64EEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 40(%rax)
	movq	-8(%rbp), %rax
	addq	$48, %rax
	movq	%rax, %rdi
	call	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC1Ev
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE63:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E, .-_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EEC1EPFbRS2_S4_E
	.set	_ZN5alloc11SortedQueueIP6task_sLm64EEC1EPFbRS2_S4_E,_ZN5alloc11SortedQueueIP6task_sLm64EEC2EPFbRS2_S4_E
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE4pushES2_,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE4pushES2_,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	.type	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_, @function
_ZN5alloc5QueueIP6task_sLm64EE4pushES2_:
.LFB65:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-24(%rbp), %rax
	addq	$40, %rax
	movq	%rax, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE5allocEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L45
	movl	$0, %eax
	jmp	.L46
.L45:
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E
	movq	-8(%rbp), %rax
	addq	$16, %rax
.L46:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE65:
	.size	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_, .-_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	.set	.LTHUNK0,_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_:
.LFB108:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK0
	.cfi_endproc
.LFE108:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv:
.LFB66:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE9peek_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L48
	movl	$0, %eax
	jmp	.L49
.L48:
	movq	-8(%rbp), %rax
	addq	$16, %rax
.L49:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE66:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv, .-_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.set	.LTHUNK1,_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv:
.LFB109:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK1
	.cfi_endproc
.LFE109:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE3popEv,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv:
.LFB67:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE8pop_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	je	.L53
	movq	-24(%rbp), %rax
	leaq	48(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	jmp	.L50
.L53:
	nop
.L50:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE67:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv, .-_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv
	.set	.LTHUNK2,_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv:
.LFB110:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK2
	.cfi_endproc
.LFE110:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE4peekEv,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE4peekEv,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE4peekEv
	.type	_ZN5alloc5QueueIP6task_sLm64EE4peekEv, @function
_ZN5alloc5QueueIP6task_sLm64EE4peekEv:
.LFB68:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE9peek_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L55
	movl	$0, %eax
	jmp	.L56
.L55:
	movq	-8(%rbp), %rax
	addq	$16, %rax
.L56:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE68:
	.size	_ZN5alloc5QueueIP6task_sLm64EE4peekEv, .-_ZN5alloc5QueueIP6task_sLm64EE4peekEv
	.set	.LTHUNK3,_ZN5alloc5QueueIP6task_sLm64EE4peekEv
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv:
.LFB111:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK3
	.cfi_endproc
.LFE111:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE3popEv,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE3popEv,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE3popEv
	.type	_ZN5alloc5QueueIP6task_sLm64EE3popEv, @function
_ZN5alloc5QueueIP6task_sLm64EE3popEv:
.LFB69:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE8pop_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	je	.L60
	movq	-24(%rbp), %rax
	leaq	40(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	jmp	.L57
.L60:
	nop
.L57:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE69:
	.size	_ZN5alloc5QueueIP6task_sLm64EE3popEv, .-_ZN5alloc5QueueIP6task_sLm64EE3popEv
	.set	.LTHUNK4,_ZN5alloc5QueueIP6task_sLm64EE3popEv
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv:
.LFB112:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK4
	.cfi_endproc
.LFE112:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_:
.LFB70:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	subq	$16, %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E
	movq	-24(%rbp), %rax
	leaq	48(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE70:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_, .-_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.set	.LTHUNK5,_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_:
.LFB113:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK5
	.cfi_endproc
.LFE113:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.type	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_, @function
_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_:
.LFB71:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	subq	$16, %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E
	movq	-24(%rbp), %rax
	leaq	40(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE71:
	.size	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_, .-_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.set	.LTHUNK6,_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_:
.LFB114:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK6
	.cfi_endproc
.LFE114:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_:
.LFB72:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-24(%rbp), %rax
	addq	$48, %rax
	movq	%rax, %rdi
	call	_ZN4PoolI4NodeIP6task_sEE5allocEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L64
	movl	$0, %eax
	jmp	.L65
.L64:
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E
	movq	-8(%rbp), %rax
	addq	$16, %rax
.L65:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE72:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_, .-_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.set	.LTHUNK7,_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_:
.LFB115:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK7
	.cfi_endproc
.LFE115:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.section	.text._ZN11SimpleQueueIP6task_sEC2Ev,"axG",@progbits,_ZN11SimpleQueueIP6task_sEC5Ev,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sEC2Ev
	.type	_ZN11SimpleQueueIP6task_sEC2Ev, @function
_ZN11SimpleQueueIP6task_sEC2Ev:
.LFB74:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	leaq	16+_ZTV11SimpleQueueIP6task_sE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 16(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 24(%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE74:
	.size	_ZN11SimpleQueueIP6task_sEC2Ev, .-_ZN11SimpleQueueIP6task_sEC2Ev
	.weak	_ZN11SimpleQueueIP6task_sEC1Ev
	.set	_ZN11SimpleQueueIP6task_sEC1Ev,_ZN11SimpleQueueIP6task_sEC2Ev
	.section	.text._ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev,"axG",@progbits,_ZN5alloc4PoolI4NodeIP6task_sELm64EEC5Ev,comdat
	.align 2
	.weak	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev
	.type	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev, @function
_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev:
.LFB77:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	-8(%rbp), %rdx
	addq	$2648, %rdx
	movq	-8(%rbp), %rcx
	leaq	88(%rcx), %rsi
	movl	$64, %ecx
	movq	%rax, %rdi
	call	_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em
	leaq	16+_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE77:
	.size	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev, .-_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev
	.weak	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC1Ev
	.set	_ZN5alloc4PoolI4NodeIP6task_sELm64EEC1Ev,_ZN5alloc4PoolI4NodeIP6task_sELm64EEC2Ev
	.section	.text._ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E,"axG",@progbits,_ZN17SimpleSortedQueueIP6task_sEC5EPFbRS1_S3_E,comdat
	.align 2
	.weak	_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E
	.type	_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E, @function
_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E:
.LFB80:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sEC2Ev
	leaq	16+_ZTV17SimpleSortedQueueIP6task_sE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 32(%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE80:
	.size	_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E, .-_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E
	.weak	_ZN17SimpleSortedQueueIP6task_sEC1EPFbRS1_S3_E
	.set	_ZN17SimpleSortedQueueIP6task_sEC1EPFbRS1_S3_E,_ZN17SimpleSortedQueueIP6task_sEC2EPFbRS1_S3_E
	.section	.text._ZN4PoolI4NodeIP6task_sEE5allocEv,"axG",@progbits,_ZN4PoolI4NodeIP6task_sEE5allocEv,comdat
	.align 2
	.weak	_ZN4PoolI4NodeIP6task_sEE5allocEv
	.type	_ZN4PoolI4NodeIP6task_sEE5allocEv, @function
_ZN4PoolI4NodeIP6task_sEE5allocEv:
.LFB82:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	addq	$24, %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L70
	movl	$0, %eax
	jmp	.L71
.L70:
	movq	-24(%rbp), %rax
	leaq	56(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	movb	$1, 8(%rax)
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	addq	$16, %rax
.L71:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE82:
	.size	_ZN4PoolI4NodeIP6task_sEE5allocEv, .-_ZN4PoolI4NodeIP6task_sEE5allocEv
	.section	.text._ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E,"axG",@progbits,_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.type	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E, @function
_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E:
.LFB83:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	testq	%rax, %rax
	jne	.L73
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-16(%rbp), %rax
	movq	$0, (%rax)
	movq	-16(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$1, 24(%rax)
	jmp	.L75
.L73:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	leaq	1(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	movq	$0, (%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 8(%rax)
.L75:
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE83:
	.size	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E, .-_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.section	.text._ZN11SimpleQueueIP6task_sE9peek_nodeEv,"axG",@progbits,_ZN11SimpleQueueIP6task_sE9peek_nodeEv,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE9peek_nodeEv
	.type	_ZN11SimpleQueueIP6task_sE9peek_nodeEv, @function
_ZN11SimpleQueueIP6task_sE9peek_nodeEv:
.LFB84:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE84:
	.size	_ZN11SimpleQueueIP6task_sE9peek_nodeEv, .-_ZN11SimpleQueueIP6task_sE9peek_nodeEv
	.section	.text._ZN11SimpleQueueIP6task_sE8pop_nodeEv,"axG",@progbits,_ZN11SimpleQueueIP6task_sE8pop_nodeEv,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE8pop_nodeEv
	.type	_ZN11SimpleQueueIP6task_sE8pop_nodeEv, @function
_ZN11SimpleQueueIP6task_sE8pop_nodeEv:
.LFB85:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L79
	movq	-8(%rbp), %rax
	jmp	.L80
.L79:
	movq	-8(%rbp), %rax
	movq	(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	movq	24(%rax), %rax
	leaq	-1(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	testq	%rax, %rax
	jne	.L81
	movq	-24(%rbp), %rax
	movq	$0, 8(%rax)
	jmp	.L82
.L81:
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	$0, 8(%rax)
.L82:
	movq	-8(%rbp), %rax
.L80:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE85:
	.size	_ZN11SimpleQueueIP6task_sE8pop_nodeEv, .-_ZN11SimpleQueueIP6task_sE8pop_nodeEv
	.section	.text._ZN4PoolI4NodeIP6task_sEE4freeEPS3_,"axG",@progbits,_ZN4PoolI4NodeIP6task_sEE4freeEPS3_,comdat
	.align 2
	.weak	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	.type	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_, @function
_ZN4PoolI4NodeIP6task_sEE4freeEPS3_:
.LFB86:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	subq	$16, %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movzbl	8(%rax), %eax
	xorl	$1, %eax
	testb	%al, %al
	je	.L84
	movl	$0, %eax
	jmp	.L85
.L84:
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	cmpq	%rax, -40(%rbp)
	je	.L86
	movl	$0, %eax
	jmp	.L85
.L86:
	movq	-40(%rbp), %rax
	addq	$56, %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L87
	movl	$0, %eax
	jmp	.L85
.L87:
	movq	-16(%rbp), %rax
	movb	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-40(%rbp), %rax
	leaq	24(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movq	%rdx, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	movl	$1, %eax
.L85:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE86:
	.size	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_, .-_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	.section	.text._ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E,"axG",@progbits,_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E
	.type	_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E, @function
_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E:
.LFB87:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	cmpq	%rax, -16(%rbp)
	jne	.L89
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	8(%rax), %rax
	testq	%rax, %rax
	jne	.L90
	movq	-8(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 16(%rax)
	jmp	.L92
.L90:
	movq	-16(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	$0, (%rax)
	jmp	.L92
.L89:
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	cmpq	%rax, -16(%rbp)
	jne	.L93
	movq	-16(%rbp), %rax
	movq	(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-8(%rbp), %rax
	movq	16(%rax), %rax
	movq	$0, 8(%rax)
	jmp	.L92
.L93:
	movq	-16(%rbp), %rax
	movq	(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	8(%rdx), %rdx
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	movq	8(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	(%rdx), %rdx
	movq	%rdx, (%rax)
.L92:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	leaq	-1(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 24(%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE87:
	.size	_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E, .-_ZN11SimpleQueueIP6task_sE11remove_nodeEP4NodeIS1_E
	.section	.text._ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E,"axG",@progbits,_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E,comdat
	.align 2
	.weak	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.type	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E, @function
_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E:
.LFB88:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	testq	%rax, %rax
	jne	.L95
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-32(%rbp), %rax
	movq	$0, (%rax)
	movq	-32(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-24(%rbp), %rax
	movq	$1, 24(%rax)
	jmp	.L102
.L95:
	movq	-24(%rbp), %rax
	movq	24(%rax), %rax
	leaq	1(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
.L99:
	cmpq	$0, -8(%rbp)
	je	.L97
	movq	-24(%rbp), %rax
	movq	32(%rax), %rax
	movq	-32(%rbp), %rdx
	leaq	16(%rdx), %rcx
	movq	-8(%rbp), %rdx
	addq	$16, %rdx
	movq	%rcx, %rsi
	movq	%rdx, %rdi
	call	*%rax
	testb	%al, %al
	jne	.L101
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
	jmp	.L99
.L101:
	nop
.L97:
	cmpq	$0, -8(%rbp)
	jne	.L100
	movq	-24(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-32(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 16(%rax)
	jmp	.L102
.L100:
	movq	-32(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-8(%rbp), %rax
	movq	(%rax), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	cmpq	%rax, -8(%rbp)
	jne	.L102
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 8(%rax)
.L102:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE88:
	.size	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E, .-_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.section	.text._ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em,"axG",@progbits,_ZN4PoolI4NodeIP6task_sEEC5EP14PoolDescriptorIS3_EPS0_IS7_Em,comdat
	.align 2
	.weak	_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em
	.type	_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em, @function
_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em:
.LFB90:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%rcx, -48(%rbp)
	leaq	16+_ZTV4PoolI4NodeIP6task_sEE(%rip), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-24(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	addq	$24, %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC1Ev
	movq	-24(%rbp), %rax
	addq	$56, %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC1Ev
	movq	$0, -8(%rbp)
.L105:
	movq	-8(%rbp), %rax
	cmpq	-48(%rbp), %rax
	jnb	.L106
	movq	-24(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	salq	$2, %rax
	addq	%rdx, %rax
	salq	$3, %rax
	leaq	(%rcx,%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rax, (%rdx)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	salq	$2, %rax
	addq	%rdx, %rax
	salq	$3, %rax
	addq	%rcx, %rax
	movb	$0, 8(%rax)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rcx
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	salq	$2, %rax
	addq	%rdx, %rax
	salq	$3, %rax
	movq	%rax, %rdi
	movq	-24(%rbp), %rax
	movq	16(%rax), %rsi
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$3, %rax
	addq	%rsi, %rax
	leaq	(%rcx,%rdi), %rdx
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	leaq	24(%rax), %rcx
	movq	-24(%rbp), %rax
	movq	16(%rax), %rsi
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	addq	%rax, %rax
	addq	%rdx, %rax
	salq	$3, %rax
	addq	%rsi, %rax
	movq	%rax, %rsi
	movq	%rcx, %rdi
	call	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	addq	$1, -8(%rbp)
	jmp	.L105
.L106:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE90:
	.size	_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em, .-_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em
	.weak	_ZN4PoolI4NodeIP6task_sEEC1EP14PoolDescriptorIS3_EPS0_IS7_Em
	.set	_ZN4PoolI4NodeIP6task_sEEC1EP14PoolDescriptorIS3_EPS0_IS7_Em,_ZN4PoolI4NodeIP6task_sEEC2EP14PoolDescriptorIS3_EPS0_IS7_Em
	.section	.text._ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv,"axG",@progbits,_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv
	.type	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv, @function
_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv:
.LFB92:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L108
	movq	-8(%rbp), %rax
	jmp	.L109
.L108:
	movq	-8(%rbp), %rax
	movq	(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-24(%rbp), %rax
	movq	24(%rax), %rax
	leaq	-1(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	testq	%rax, %rax
	jne	.L110
	movq	-24(%rbp), %rax
	movq	$0, 8(%rax)
	jmp	.L111
.L110:
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	$0, 8(%rax)
.L111:
	movq	-8(%rbp), %rax
.L109:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE92:
	.size	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv, .-_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE8pop_nodeEv
	.section	.text._ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E,"axG",@progbits,_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	.type	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E, @function
_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E:
.LFB93:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	testq	%rax, %rax
	jne	.L113
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 8(%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-16(%rbp), %rax
	movq	$0, (%rax)
	movq	-16(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$1, 24(%rax)
	jmp	.L115
.L113:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	leaq	1(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	movq	$0, (%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 8(%rax)
.L115:
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE93:
	.size	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E, .-_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	.section	.text._ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev,"axG",@progbits,_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC5Ev,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev
	.type	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev, @function
_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev:
.LFB95:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	leaq	16+_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE(%rip), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 16(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 24(%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE95:
	.size	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev, .-_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev
	.weak	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC1Ev
	.set	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC1Ev,_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEEC2Ev
	.weak	_ZTVN5alloc11SortedQueueIP6task_sLm64EEE
	.section	.data.rel.ro.local._ZTVN5alloc11SortedQueueIP6task_sLm64EEE,"awG",@progbits,_ZTVN5alloc11SortedQueueIP6task_sLm64EEE,comdat
	.align 8
	.type	_ZTVN5alloc11SortedQueueIP6task_sLm64EEE, @object
	.size	_ZTVN5alloc11SortedQueueIP6task_sLm64EEE, 136
_ZTVN5alloc11SortedQueueIP6task_sLm64EEE:
	.quad	0
	.quad	_ZTIN5alloc11SortedQueueIP6task_sLm64EEE
	.quad	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE3popEv
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.quad	_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.quad	-40
	.quad	_ZTIN5alloc11SortedQueueIP6task_sLm64EEE
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4pushES2_
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE3popEv
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4peekEv
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE6removeEPS2_
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.quad	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.weak	_ZTV17SimpleSortedQueueIP6task_sE
	.section	.data.rel.ro.local._ZTV17SimpleSortedQueueIP6task_sE,"awG",@progbits,_ZTV17SimpleSortedQueueIP6task_sE,comdat
	.align 8
	.type	_ZTV17SimpleSortedQueueIP6task_sE, @object
	.size	_ZTV17SimpleSortedQueueIP6task_sE, 24
_ZTV17SimpleSortedQueueIP6task_sE:
	.quad	0
	.quad	_ZTI17SimpleSortedQueueIP6task_sE
	.quad	_ZN17SimpleSortedQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.weak	_ZTVN5alloc5QueueIP6task_sLm64EEE
	.section	.data.rel.ro.local._ZTVN5alloc5QueueIP6task_sLm64EEE,"awG",@progbits,_ZTVN5alloc5QueueIP6task_sLm64EEE,comdat
	.align 8
	.type	_ZTVN5alloc5QueueIP6task_sLm64EEE, @object
	.size	_ZTVN5alloc5QueueIP6task_sLm64EEE, 136
_ZTVN5alloc5QueueIP6task_sLm64EEE:
	.quad	0
	.quad	_ZTIN5alloc5QueueIP6task_sLm64EEE
	.quad	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.quad	_ZN5alloc5QueueIP6task_sLm64EE4pushES2_
	.quad	_ZN5alloc5QueueIP6task_sLm64EE3popEv
	.quad	_ZN5alloc5QueueIP6task_sLm64EE4peekEv
	.quad	_ZN5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.quad	_ZN5alloc5QueueIP6task_sLm64EE4sizeEv
	.quad	_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv
	.quad	-32
	.quad	_ZTIN5alloc5QueueIP6task_sLm64EEE
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE4pushES2_
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE3popEv
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE4peekEv
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE6removeEPS2_
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv
	.quad	_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv
	.weak	_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE
	.section	.data.rel.ro.local._ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE,"awG",@progbits,_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE,comdat
	.align 8
	.type	_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE, @object
	.size	_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE, 24
_ZTVN5alloc4PoolI4NodeIP6task_sELm64EEE:
	.quad	0
	.quad	_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE
	.quad	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	.weak	_ZTV4PoolI4NodeIP6task_sEE
	.section	.data.rel.ro.local._ZTV4PoolI4NodeIP6task_sEE,"awG",@progbits,_ZTV4PoolI4NodeIP6task_sEE,comdat
	.align 8
	.type	_ZTV4PoolI4NodeIP6task_sEE, @object
	.size	_ZTV4PoolI4NodeIP6task_sEE, 24
_ZTV4PoolI4NodeIP6task_sEE:
	.quad	0
	.quad	_ZTI4PoolI4NodeIP6task_sEE
	.quad	_ZN4PoolI4NodeIP6task_sEE4freeEPS3_
	.weak	_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE
	.section	.data.rel.ro.local._ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,"awG",@progbits,_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,comdat
	.align 8
	.type	_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, @object
	.size	_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, 24
_ZTV11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE:
	.quad	0
	.quad	_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE
	.quad	_ZN11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE9push_nodeEPS1_IS6_E
	.weak	_ZTV5QueueIP6task_sE
	.section	.data.rel.ro._ZTV5QueueIP6task_sE,"awG",@progbits,_ZTV5QueueIP6task_sE,comdat
	.align 8
	.type	_ZTV5QueueIP6task_sE, @object
	.size	_ZTV5QueueIP6task_sE, 64
_ZTV5QueueIP6task_sE:
	.quad	0
	.quad	_ZTI5QueueIP6task_sE
	.quad	__cxa_pure_virtual
	.quad	__cxa_pure_virtual
	.quad	__cxa_pure_virtual
	.quad	__cxa_pure_virtual
	.quad	__cxa_pure_virtual
	.quad	__cxa_pure_virtual
	.weak	_ZTV11SimpleQueueIP6task_sE
	.section	.data.rel.ro.local._ZTV11SimpleQueueIP6task_sE,"awG",@progbits,_ZTV11SimpleQueueIP6task_sE,comdat
	.align 8
	.type	_ZTV11SimpleQueueIP6task_sE, @object
	.size	_ZTV11SimpleQueueIP6task_sE, 24
_ZTV11SimpleQueueIP6task_sE:
	.quad	0
	.quad	_ZTI11SimpleQueueIP6task_sE
	.quad	_ZN11SimpleQueueIP6task_sE9push_nodeEP4NodeIS1_E
	.weak	_ZTIN5alloc11SortedQueueIP6task_sLm64EEE
	.section	.data.rel.ro._ZTIN5alloc11SortedQueueIP6task_sLm64EEE,"awG",@progbits,_ZTIN5alloc11SortedQueueIP6task_sLm64EEE,comdat
	.align 8
	.type	_ZTIN5alloc11SortedQueueIP6task_sLm64EEE, @object
	.size	_ZTIN5alloc11SortedQueueIP6task_sLm64EEE, 56
_ZTIN5alloc11SortedQueueIP6task_sLm64EEE:
	.quad	_ZTVN10__cxxabiv121__vmi_class_type_infoE+16
	.quad	_ZTSN5alloc11SortedQueueIP6task_sLm64EEE
	.long	0
	.long	2
	.quad	_ZTI17SimpleSortedQueueIP6task_sE
	.quad	2
	.quad	_ZTI5QueueIP6task_sE
	.quad	10242
	.weak	_ZTSN5alloc11SortedQueueIP6task_sLm64EEE
	.section	.rodata._ZTSN5alloc11SortedQueueIP6task_sLm64EEE,"aG",@progbits,_ZTSN5alloc11SortedQueueIP6task_sLm64EEE,comdat
	.align 32
	.type	_ZTSN5alloc11SortedQueueIP6task_sLm64EEE, @object
	.size	_ZTSN5alloc11SortedQueueIP6task_sLm64EEE, 37
_ZTSN5alloc11SortedQueueIP6task_sLm64EEE:
	.string	"N5alloc11SortedQueueIP6task_sLm64EEE"
	.weak	_ZTI17SimpleSortedQueueIP6task_sE
	.section	.data.rel.ro._ZTI17SimpleSortedQueueIP6task_sE,"awG",@progbits,_ZTI17SimpleSortedQueueIP6task_sE,comdat
	.align 8
	.type	_ZTI17SimpleSortedQueueIP6task_sE, @object
	.size	_ZTI17SimpleSortedQueueIP6task_sE, 24
_ZTI17SimpleSortedQueueIP6task_sE:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTS17SimpleSortedQueueIP6task_sE
	.quad	_ZTI11SimpleQueueIP6task_sE
	.weak	_ZTS17SimpleSortedQueueIP6task_sE
	.section	.rodata._ZTS17SimpleSortedQueueIP6task_sE,"aG",@progbits,_ZTS17SimpleSortedQueueIP6task_sE,comdat
	.align 16
	.type	_ZTS17SimpleSortedQueueIP6task_sE, @object
	.size	_ZTS17SimpleSortedQueueIP6task_sE, 30
_ZTS17SimpleSortedQueueIP6task_sE:
	.string	"17SimpleSortedQueueIP6task_sE"
	.weak	_ZTIN5alloc5QueueIP6task_sLm64EEE
	.section	.data.rel.ro._ZTIN5alloc5QueueIP6task_sLm64EEE,"awG",@progbits,_ZTIN5alloc5QueueIP6task_sLm64EEE,comdat
	.align 8
	.type	_ZTIN5alloc5QueueIP6task_sLm64EEE, @object
	.size	_ZTIN5alloc5QueueIP6task_sLm64EEE, 56
_ZTIN5alloc5QueueIP6task_sLm64EEE:
	.quad	_ZTVN10__cxxabiv121__vmi_class_type_infoE+16
	.quad	_ZTSN5alloc5QueueIP6task_sLm64EEE
	.long	0
	.long	2
	.quad	_ZTI11SimpleQueueIP6task_sE
	.quad	2
	.quad	_ZTI5QueueIP6task_sE
	.quad	8194
	.weak	_ZTSN5alloc5QueueIP6task_sLm64EEE
	.section	.rodata._ZTSN5alloc5QueueIP6task_sLm64EEE,"aG",@progbits,_ZTSN5alloc5QueueIP6task_sLm64EEE,comdat
	.align 16
	.type	_ZTSN5alloc5QueueIP6task_sLm64EEE, @object
	.size	_ZTSN5alloc5QueueIP6task_sLm64EEE, 30
_ZTSN5alloc5QueueIP6task_sLm64EEE:
	.string	"N5alloc5QueueIP6task_sLm64EEE"
	.weak	_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE
	.section	.data.rel.ro._ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE,"awG",@progbits,_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE,comdat
	.align 8
	.type	_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE, @object
	.size	_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE, 24
_ZTIN5alloc4PoolI4NodeIP6task_sELm64EEE:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE
	.quad	_ZTI4PoolI4NodeIP6task_sEE
	.weak	_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE
	.section	.rodata._ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE,"aG",@progbits,_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE,comdat
	.align 32
	.type	_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE, @object
	.size	_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE, 36
_ZTSN5alloc4PoolI4NodeIP6task_sELm64EEE:
	.string	"N5alloc4PoolI4NodeIP6task_sELm64EEE"
	.weak	_ZTI4PoolI4NodeIP6task_sEE
	.section	.data.rel.ro._ZTI4PoolI4NodeIP6task_sEE,"awG",@progbits,_ZTI4PoolI4NodeIP6task_sEE,comdat
	.align 8
	.type	_ZTI4PoolI4NodeIP6task_sEE, @object
	.size	_ZTI4PoolI4NodeIP6task_sEE, 16
_ZTI4PoolI4NodeIP6task_sEE:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS4PoolI4NodeIP6task_sEE
	.weak	_ZTS4PoolI4NodeIP6task_sEE
	.section	.rodata._ZTS4PoolI4NodeIP6task_sEE,"aG",@progbits,_ZTS4PoolI4NodeIP6task_sEE,comdat
	.align 16
	.type	_ZTS4PoolI4NodeIP6task_sEE, @object
	.size	_ZTS4PoolI4NodeIP6task_sEE, 23
_ZTS4PoolI4NodeIP6task_sEE:
	.string	"4PoolI4NodeIP6task_sEE"
	.weak	_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE
	.section	.data.rel.ro._ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,"awG",@progbits,_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,comdat
	.align 8
	.type	_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, @object
	.size	_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, 16
_ZTI11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE
	.weak	_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE
	.section	.rodata._ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,"aG",@progbits,_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE,comdat
	.align 32
	.type	_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, @object
	.size	_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE, 50
_ZTS11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE:
	.string	"11SimpleQueueIP14PoolDescriptorI4NodeIP6task_sEEE"
	.weak	_ZTI5QueueIP6task_sE
	.section	.data.rel.ro._ZTI5QueueIP6task_sE,"awG",@progbits,_ZTI5QueueIP6task_sE,comdat
	.align 8
	.type	_ZTI5QueueIP6task_sE, @object
	.size	_ZTI5QueueIP6task_sE, 16
_ZTI5QueueIP6task_sE:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS5QueueIP6task_sE
	.weak	_ZTS5QueueIP6task_sE
	.section	.rodata._ZTS5QueueIP6task_sE,"aG",@progbits,_ZTS5QueueIP6task_sE,comdat
	.align 16
	.type	_ZTS5QueueIP6task_sE, @object
	.size	_ZTS5QueueIP6task_sE, 17
_ZTS5QueueIP6task_sE:
	.string	"5QueueIP6task_sE"
	.weak	_ZTI11SimpleQueueIP6task_sE
	.section	.data.rel.ro._ZTI11SimpleQueueIP6task_sE,"awG",@progbits,_ZTI11SimpleQueueIP6task_sE,comdat
	.align 8
	.type	_ZTI11SimpleQueueIP6task_sE, @object
	.size	_ZTI11SimpleQueueIP6task_sE, 16
_ZTI11SimpleQueueIP6task_sE:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS11SimpleQueueIP6task_sE
	.weak	_ZTS11SimpleQueueIP6task_sE
	.section	.rodata._ZTS11SimpleQueueIP6task_sE,"aG",@progbits,_ZTS11SimpleQueueIP6task_sE,comdat
	.align 16
	.type	_ZTS11SimpleQueueIP6task_sE, @object
	.size	_ZTS11SimpleQueueIP6task_sE, 24
_ZTS11SimpleQueueIP6task_sE:
	.string	"11SimpleQueueIP6task_sE"
	.text
	.type	_Z41__static_initialization_and_destruction_0ii, @function
_Z41__static_initialization_and_destruction_0ii:
.LFB97:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$1, -4(%rbp)
	jne	.L119
	cmpl	$65535, -8(%rbp)
	jne	.L119
	leaq	_ZL7ready_q(%rip), %rdi
	call	_ZN5alloc5QueueIP6task_sLm64EEC1Ev
	leaq	_Z10sleep_sortRP6task_sS1_(%rip), %rsi
	leaq	_ZL7sleep_q(%rip), %rdi
	call	_ZN5alloc11SortedQueueIP6task_sLm64EEC1EPFbRS2_S4_E
.L119:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE97:
	.size	_Z41__static_initialization_and_destruction_0ii, .-_Z41__static_initialization_and_destruction_0ii
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv:
.LFB98:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE9num_nodesEv
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE98:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv, .-_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.set	.LTHUNK8,_ZN5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv:
.LFB116:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK8
	.cfi_endproc
.LFE116:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE4sizeEv
	.section	.text._ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv,"axG",@progbits,_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv,comdat
	.align 2
	.weak	_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.type	_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv, @function
_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv:
.LFB99:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE4headEv
	movq	%rax, %rdx
	leaq	-16(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN13QueueIteratorIP6task_sEC1EP4NodeIS1_E
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L124
	call	__stack_chk_fail@PLT
.L124:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE99:
	.size	_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv, .-_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.set	.LTHUNK9,_ZN5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.weak	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.type	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv, @function
_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv:
.LFB117:
	.cfi_startproc
	subq	$40, %rdi
	jmp	.LTHUNK9
	.cfi_endproc
.LFE117:
	.size	_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv, .-_ZThn40_N5alloc11SortedQueueIP6task_sLm64EE8iteratorEv
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE4sizeEv,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE4sizeEv,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE4sizeEv
	.type	_ZN5alloc5QueueIP6task_sLm64EE4sizeEv, @function
_ZN5alloc5QueueIP6task_sLm64EE4sizeEv:
.LFB100:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE9num_nodesEv
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE100:
	.size	_ZN5alloc5QueueIP6task_sLm64EE4sizeEv, .-_ZN5alloc5QueueIP6task_sLm64EE4sizeEv
	.set	.LTHUNK10,_ZN5alloc5QueueIP6task_sLm64EE4sizeEv
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv:
.LFB118:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK10
	.cfi_endproc
.LFE118:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE4sizeEv
	.section	.text._ZN5alloc5QueueIP6task_sLm64EE8iteratorEv,"axG",@progbits,_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv,comdat
	.align 2
	.weak	_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv
	.type	_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv, @function
_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv:
.LFB101:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN11SimpleQueueIP6task_sE4headEv
	movq	%rax, %rdx
	leaq	-16(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZN13QueueIteratorIP6task_sEC1EP4NodeIS1_E
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L129
	call	__stack_chk_fail@PLT
.L129:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE101:
	.size	_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv, .-_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv
	.set	.LTHUNK11,_ZN5alloc5QueueIP6task_sLm64EE8iteratorEv
	.weak	_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv
	.type	_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv, @function
_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv:
.LFB119:
	.cfi_startproc
	subq	$32, %rdi
	jmp	.LTHUNK11
	.cfi_endproc
.LFE119:
	.size	_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv, .-_ZThn32_N5alloc5QueueIP6task_sLm64EE8iteratorEv
	.section	.text._ZN11SimpleQueueIP6task_sE9num_nodesEv,"axG",@progbits,_ZN11SimpleQueueIP6task_sE9num_nodesEv,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE9num_nodesEv
	.type	_ZN11SimpleQueueIP6task_sE9num_nodesEv, @function
_ZN11SimpleQueueIP6task_sE9num_nodesEv:
.LFB102:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE102:
	.size	_ZN11SimpleQueueIP6task_sE9num_nodesEv, .-_ZN11SimpleQueueIP6task_sE9num_nodesEv
	.section	.text._ZN11SimpleQueueIP6task_sE4headEv,"axG",@progbits,_ZN11SimpleQueueIP6task_sE4headEv,comdat
	.align 2
	.weak	_ZN11SimpleQueueIP6task_sE4headEv
	.type	_ZN11SimpleQueueIP6task_sE4headEv, @function
_ZN11SimpleQueueIP6task_sE4headEv:
.LFB103:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE103:
	.size	_ZN11SimpleQueueIP6task_sE4headEv, .-_ZN11SimpleQueueIP6task_sE4headEv
	.section	.text._ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E,"axG",@progbits,_ZN13QueueIteratorIP6task_sEC5EP4NodeIS1_E,comdat
	.align 2
	.weak	_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E
	.type	_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E, @function
_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E:
.LFB105:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, (%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE105:
	.size	_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E, .-_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E
	.weak	_ZN13QueueIteratorIP6task_sEC1EP4NodeIS1_E
	.set	_ZN13QueueIteratorIP6task_sEC1EP4NodeIS1_E,_ZN13QueueIteratorIP6task_sEC2EP4NodeIS1_E
	.text
	.type	_GLOBAL__sub_I__Z10sleep_sortRP6task_sS1_, @function
_GLOBAL__sub_I__Z10sleep_sortRP6task_sS1_:
.LFB107:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$65535, %esi
	movl	$1, %edi
	call	_Z41__static_initialization_and_destruction_0ii
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE107:
	.size	_GLOBAL__sub_I__Z10sleep_sortRP6task_sS1_, .-_GLOBAL__sub_I__Z10sleep_sortRP6task_sS1_
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I__Z10sleep_sortRP6task_sS1_
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
