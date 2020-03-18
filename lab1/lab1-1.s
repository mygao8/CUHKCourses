	.globl main	
	.data 
var1: .word 15
var2: .word 19
	.text
	main:
	li	$v0, 1	# system call code for print_int
	la	$a0, var1	# addr of var1
	syscall		# print int var1
	la	$a0, var2 # addr of var2
	syscall		# print int var2
	lw	$t0, var1	# load var1 to $t0
	addi	$t0, 1	# $t0 = $t0 + 1
	sw 	$t0, var1	# store $t0 to var1
	move	$a0, $t0	# $a0 = $t0
	syscall		# print var1 after increment by 1
	lw	$t0, var2	# load var2 to $t0
	sll	$t0, $t0, 2 # $t0 = $t0 << 2, i.e. multiply $t0 by 4
	sw 	$t0, var2	# store $t0 to var1	
	move	$a0, $t0	# $a0 = $t0
	syscall		# print var2 after multiplying var2 by 4
	lw	$t1, var1	# load var1 to $t1, and var2 is in $t0 now
	sw	$t1, var2	# store value of var1 into var2
	sw 	$t0, var1	# store value of var2 into var1
	lw	$a0, var1	# load var1 to $a0
	syscall		# print var1 after swap
	lw 	$a0, var2	# load var2 to $a0
	syscall		# print var2 after swap
	li	$v0, 10	# system call code for exit
	syscall  		# exit