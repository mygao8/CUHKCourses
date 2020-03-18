	.globl main	
	.data
A: .word -1 22 8 35 5 4 11 2 1 78

    .text
    swap: 
        sll $t1, $a1, 2     # $a1 stores a, $t1=$a1*4 is the offset of A[i] in array A
        add $t1, $a0, $t1   # $a0 stores the addr of the 1st element in array, $t1 is the addr of i-th element
        sll $t2, $a2, 2     # $a2 stores b, $t2=$a2*4 is the offset of A[j] in array A 
        add $t2, $a0, $t2   # $t2 is the addr of j-th element
        lw  $t4, 0($t1)     # $t4 = A[i]
        lw  $t3, 0($t2)     # $t3 = A[j]
        sw  $t3, 0($t1)     # A[i] = A[j] 
        sw  $t4, 0($t2)     # A[j] = $t4 (the original A[i])
        jr   $ra            # return to caller

    partition:
        addi $sp, $sp, -28  # save 7 registers into stack
        sw $ra, 24($sp)     # save the return address into stack
        sw $s5, 20($sp)     # the following 3 registers will be used to save arguments A, lo, hi
        sw $s4, 16($sp)
        sw $s3, 12($sp)      
        sw $s2, 8($sp)      # the following 3 registers will be used to save local variables i, j, pivot
        sw $s1, 4($sp)      
        sw $s0, 0($sp)

        move $s5, $a2       # copy parameter $a2(hi) into $s5 (save $a2)
        move $s4, $a1       # copy parameter $a1(lo) into $s4 (save $a1)
        move $s3, $a0       # copy parameter $a0(A)  into $s3 (save $a0)
        addi $s0, $s4, -1   # i = lo-1
        move $s1, $s4       # j = lo
        sll  $t0, $s5, 2    # $t0 = hi*4
        add  $t0, $s3, $t0  # $t0 = A + hi*4
        lw   $s2, 0($t0)    # $s2 = A[hi] (pivot = A[hi] = 8, the orginal 3rd element in array)
        addi $t0, $s5, -1   # $t0 = $s5-1 (hi-1)
    for:
        slt  $t1, $t0, $s1 # $t1=1 if $t0<$s1 (hi-1<j)
        bne  $t1, $zero, exit # go to exit if j > hi-1
        sll  $t2, $s1, 2    # $t2 = j*4
        add  $t2, $s3, $t2  # $t2 = A + j*4
        lw   $t3, 0($t2)    # $t3 = A[j]
        slt  $t1, $s2, $t3  # $t1=1 if $s2<$t1 (pivot<A[j])
        bne  $t1, $zero, else # go to else if A[j] > pivot
        addi $s0, $s0, 1    # i += 1
        move $a0, $s3       # 1st parameter of swap is A
        move $a1, $s0       # 2nd parameter of swap is i
        move $a2, $s1       # 3rd parameter of swap is j
        jal  swap           # swap (A, i, j), swap A[i] and A[j]
    else:
        addi $s1, $s1, 1    # j+=1
        j    for            # jump to test of for loop
    exit:
        move $a0, $s3       # 1st parameter of swap is A
        addi $a1, $s0, 1    # 2nd parameter of swap is i+1
        move $a2, $s5       # 3rd parameter of swap is hi
        jal  swap           # swap (A, i+1, hi), swap A[i+1] and A[hi]
        addi $v0, $s0, 1    # return i+1 in $vo          
        lw $ra, 24($sp)     # restore registers
        lw $s5, 20($sp)     
        lw $s4, 16($sp)
        lw $s3, 12($sp)      
        lw $s2, 8($sp)      
        lw $s1, 4($sp)      
        lw $s0, 0($sp)
        addi $sp, $sp, 28
        jr   $ra            # return to caller

    main:
        la   $a0, A         # 1st parameter of swap is A
        addi $a1, $zero, 2  # 2nd parameter of swap is 2 (the 3rd element, 8)
        addi $a2, $zero, 9  # 3rd parameter of swap is hi (=9)
        jal  swap           # swap (A, 2, hi), swap A[2] and A[hi]

        la   $a0, A         # 1st parameter of partition is A
        move $a1, $zero     # 2nd parameter of partition is lo (=0)
        addi $a2, $zero, 9  # 3rd parameter of partition is hi (=9)
        jal  partition      # partition(A, lo, hi)
        li   $v0, 10        # system call code for exit
        syscall             # exit