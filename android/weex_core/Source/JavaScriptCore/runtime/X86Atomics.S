	.section	.text.__atomic_store_8 ,"ax",@progbits
	.globl	__atomic_store_8
	.p2align	4, 0x90
	.type	__atomic_store_8,@function
__atomic_store_8:
	.cfi_startproc
    .cfi_def_cfa_offset 4
    movl 4(%esp), %eax
    movq 8(%esp), %xmm0
    movq %xmm0, (%eax)
    mfence
    ret
	.cfi_endproc
.Lfunc_end0:
	.size __atomic_store_8, .-__atomic_store_8

	.section	.text.__atomic_load_8 ,"ax",@progbits
	.globl	__atomic_load_8
	.p2align	4, 0x90
	.type	__atomic_load_8,@function
__atomic_load_8:
	.cfi_startproc
	subl	$12, %esp
	.cfi_def_cfa_offset 16
	movl	16(%esp), %eax
	movq	(%eax), %xmm0
	movq	%xmm0, (%esp)
	movl	(%esp), %eax
	movl	4(%esp), %edx
	addl	$12, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.Lfunc_end1:
	.size __atomic_load_8, .-__atomic_load_8
