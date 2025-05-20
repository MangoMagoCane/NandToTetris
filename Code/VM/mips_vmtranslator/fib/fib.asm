.data
temp:		.word	0:8
static:		.word	0:240
data_offset:	.word	0:32768

.text
	li	$t0, 0xffff
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- bootstrapping call to Sys.init -----
	j	Sys.init
#   --- file Main ---
# ----- function Main.main 1 ------
Main.main:
	subi	$sp, $sp, 4
	sw	$zero, 4($sp)
# ----- push constant 1 -----
	li	$t0, 1
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- pop local 0 -----
	sub	$t0, $s0, 0
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, ($t0)
# ----- label L1 ------
Main.main$L1:
# ----- push local 0 -----
	sub	$t0, $s0, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 21 -----
	li	$t0, 21
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- comp: lt -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	li	$t3, 0
	blt	$t0, $t1, Main$1
	not	$t3, $t3
	Main$1:
	not	$t3, $t3
	addi	$sp, $sp, 4
	sw	$t3, 4($sp)
# ----- unary: not -----
	lw	$t0, 4($sp)
	not	$t0, $t0
	sw	$t0, 4($sp)
# ----- if-goto L2 ------
	addi	$sp, $sp, 4
	lw	$t0, ($sp)
	bnez	$t0, Main.main$L2
# ----- push local 0 -----
	sub	$t0, $s0, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- call Main.rec 1 ------
	la	$t0, Main.rec
	li	$t1, 4
	jal	call_jmp
# ----- call Output.printInt 1 ------
	la	$t0, Output.printInt
	li	$t1, 4
	jal	call_jmp
# ----- pop temp 0 -----
	li	$t0, 0
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, temp($t0)
# ----- push local 0 -----
	sub	$t0, $s0, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 1 -----
	li	$t0, 1
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- binary: add -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	add	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- pop local 0 -----
	sub	$t0, $s0, 0
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, ($t0)
# ----- goto L1 ------
	j	Main.main$L1
# ----- label L2 ------
Main.main$L2:
# ----- push constant 0 -----
	li	$t0, 0
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- return ------
	j	return_jmp
# ----- function Main.rec 0 ------
Main.rec:
# ----- push argument 0 -----
	sub	$t0, $s1, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 2 -----
	li	$t0, 2
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- comp: lt -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	li	$t3, 0
	blt	$t0, $t1, Main$2
	not	$t3, $t3
	Main$2:
	not	$t3, $t3
	addi	$sp, $sp, 4
	sw	$t3, 4($sp)
# ----- unary: not -----
	lw	$t0, 4($sp)
	not	$t0, $t0
	sw	$t0, 4($sp)
# ----- if-goto L3 ------
	addi	$sp, $sp, 4
	lw	$t0, ($sp)
	bnez	$t0, Main.rec$L3
# ----- push argument 0 -----
	sub	$t0, $s1, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- return ------
	j	return_jmp
# ----- label L3 ------
Main.rec$L3:
# ----- push argument 0 -----
	sub	$t0, $s1, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 2 -----
	li	$t0, 2
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- binary: sub -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	sub	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- call Main.rec 1 ------
	la	$t0, Main.rec
	li	$t1, 4
	jal	call_jmp
# ----- push argument 0 -----
	sub	$t0, $s1, 0
	lw	$t0, ($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 1 -----
	li	$t0, 1
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- binary: sub -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	sub	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- call Main.rec 1 ------
	la	$t0, Main.rec
	li	$t1, 4
	jal	call_jmp
# ----- binary: add -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	add	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- return ------
	j	return_jmp
# ----- os -----
#   --- Sys ---
Sys.init:
	move	$s0, $sp
	la	$t0, Main.main
	li	$t1, 0
	jal	call_jmp
	li	$v0, 10
	syscall
#   --- Output ---
Output.printInt:
	lw	$a0, ($s1)
	li	$v0, 1
	syscall
	li	$a0, 10	# newline
	li	$v0, 11	#print byte
	syscall
	j	return_jmp
return_jmp:
	move	$t0, $s0 #return_jmp
	lw	$t1, 20($t0)
	lw	$t2, 4($sp)
	sw	$t2, ($s1)
	subi	$sp, $s1, 4
	lw	$s0, 16($t0)
	lw	$s1, 12($t0)
	lw	$s2, 8($t0)
	lw	$s3, 4($t0)
	jr	$t1
call_jmp:
	subi	$sp, $sp, 20 # call_jmp
	sw	$ra, 20($sp)
	sw	$s0, 16($sp)
	sw	$s1, 12($sp)
	sw	$s2, 8($sp)
	sw	$s3, 4($sp)
	addi	$t1, $t1, 20
	add	$s1, $sp, $t1
	move	$s0, $sp
	jr	$t0
