
        .section .text

        .globl main
main:
        pushq    %rbp                /* enter    $0 */
        movq     %rsp, %rbp
        subq     $96, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        movq     $0, -24(%rbp)       /* mov_l    vr12, $67 */
        movq     $0, %r11            /* Clear temp register */
        movl     $67, %r11d          /* Moving src to temp */
        movl     %r11d, -24(%rbp)    /* Moving temp to dst */
        nop                          /* neg_l    vr13, vr12 */
        cmpb     -32(%rbp), $0       /* Compare Target with 0 */
        setne    -32(%rbp)           /* Store Result Flag in DST */
        movq     $0, -16(%rbp)       /* mov_b    vr11, vr13 */
        movq     $0, %r11            /* Clear temp register */
        movb     -32(%rbp), %r11b    /* Moving src to temp */
        movb     %r11b, -16(%rbp)    /* Moving temp to dst */
        movq     $0, -40(%rbp)       /* mov_b    vr14, vr11 */
        movq     $0, %r11            /* Clear temp register */
        movb     -16(%rbp), %r11b    /* Moving src to temp */
        movb     %r11b, -40(%rbp)    /* Moving temp to dst */
        movq     $0, %rdi            /* mov_b    vr1, vr14 */
        movq     $0, %r11            /* Clear temp register */
        movb     -40(%rbp), %r11b    /* Moving src to temp */
        movb     %r11b, %dil         /* Moving temp to dst */
        call     print_i32           /* call     print_i32 */
        movq     $0, -48(%rbp)       /* spill_b  vr15, vr0 */
        movq     $0, %r11            /* Clear temp register */
        movb     %al, %r11b          /* Moving src to temp */
        movb     %r11b, -48(%rbp)    /* Moving temp to dst */
        call     print_nl            /* call     print_nl */
        movq     $0, -56(%rbp)       /* spill_b  vr16, vr0 */
        movq     $0, %r11            /* Clear temp register */
        movb     %al, %r11b          /* Moving src to temp */
        movb     %r11b, -56(%rbp)    /* Moving temp to dst */
        movq     $0, -64(%rbp)       /* mov_l    vr17, $0 */
        movq     $0, %r11            /* Clear temp register */
        movl     $0, %r11d           /* Moving src to temp */
        movl     %r11d, -64(%rbp)    /* Moving temp to dst */
        movq     $0, %rax            /* mov_l    vr0, vr17 */
        movq     $0, %r11            /* Clear temp register */
        movl     -64(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lmain_return:
        popq     %r15                /* leave    $0 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $96, %rsp
        popq     %rbp
        ret                          /* ret       */