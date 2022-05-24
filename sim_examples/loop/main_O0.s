	.text
	.file	"main.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -4
	STi r3 r2 3
	ADDi r2 r3 4
	MOVli r4 0
	STi r4 r3 -2
	STi r4 r3 -3
	STi r4 r3 -4
	B .LBB0_1
.LBB0_1:
	LDi r4 r3 -4
	MOVli r9 99
	B.GT r4 r9 .LBB0_4
	B .LBB0_2
.LBB0_2:
	LDi r9 r3 -4
	LDi r4 r3 -3
	ADD r4 r4 r9
	STi r4 r3 -3
	B .LBB0_3
.LBB0_3:
	LDi r4 r3 -4
	ADDi r4 r4 1
	STi r4 r3 -4
	B .LBB0_1
.LBB0_4:
	LDi r9 r3 -3
	LDi r3 r2 3
	ADDi r2 r2 1
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git 572ed058aafd62e5fd6b9ef6f56e7e9f7d7ae2b5)"
	.section	".note.GNU-stack","",@progbits
