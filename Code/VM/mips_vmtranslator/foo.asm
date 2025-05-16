.data
temp:		.word	0:8
static:		.word	0:240
data_offset: .word 0

.text
	li	$t0, 0xffff
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 111 -----
	li	$t0, 111
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 333 -----
	li	$t0, 333
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push constant 888 -----
	li	$t0, 888
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- pop static 8 -----
	li	$t0, 8
	sll	$t0, $t0, 2
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, static($t0)
# ----- pop static 3 -----
	li	$t0, 3
	sll	$t0, $t0, 2
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, static($t0)
# ----- pop static 1 -----
	li	$t0, 1
	sll	$t0, $t0, 2
	addi	$sp, $sp, 4
	lw	$t1, ($sp)
	sw	$t1, static($t0)
# ----- push static 3 -----
	li	$t0, 3
	sll	$t0, $t0, 2
	lw	$t0, static($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- push static 1 -----
	li	$t0, 1
	sll	$t0, $t0, 2
	lw	$t0, static($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- binary: sub -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	sub	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- push static 8 -----
	li	$t0, 8
	sll	$t0, $t0, 2
	lw	$t0, static($t0)
	sw	$t0, ($sp)
	subi	$sp, $sp, 4
# ----- binary: add -----
	lw	$t0, 8($sp)
	lw	$t1, 4($sp)
	add	$t0, $t0, $t1
	addi	$sp, $sp, 4
	sw	$t0, 4($sp)
# ----- end_of_program -----
	lw	$a0, 4($sp)
	li	$v0, 1
	syscall
	li	$v0, 10
	syscall
