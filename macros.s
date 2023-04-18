	.file	"macros.cpp"
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
	.globl	sched_jump
	.bss
	.align 32
	.type	sched_jump, @object
	.size	sched_jump, 33792
sched_jump:
	.zero	33792
	.globl	sched_dispatched
	.align 4
	.type	sched_dispatched, @object
	.size	sched_dispatched, 4
sched_dispatched:
	.zero	4
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
