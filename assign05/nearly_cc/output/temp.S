
        .section .text

        .globl main
main:
        pushq    %rbp                /* enter    $8 */
        movq     %rsp, %rbp
        subq     $112, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        leaq     -8(%rbp), %r12      /* localaddr vr12, $0 */
        movq     %r12, -40(%rbp)     /* Moving temp to dst */
        leaq     -48(%rbp), %r12     /* localaddr vr13, vr12 */
        movq     %r12, -48(%rbp)     /* Moving temp to dst */
        movq     $0, -64(%rbp)       /* mov_q    vr15, $0 */
        movq     $0, %r11            /* Clear temp register */
        movq     $0, %r11            /* Moving src to temp */
        movq     %r11, -64(%rbp)     /* Moving temp to dst */
        nop                          /* add_q    vr16, vr15, vr13 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -64(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -48(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -72(%rbp)     /* Moving temp to dest */
        movq     $0, -80(%rbp)       /* mov_l    vr17, $2 */
        movq     $0, %r11            /* Clear temp register */
        movl     $2, %r11d           /* Moving src to temp */
        movl     %r11d, -80(%rbp)    /* Moving temp to dst */
        movq     $0, %r9             /* mov_l    (vr16), vr17 */
        movq     -72(%rbp), %r9      /* Move Address to tmp register */
        movq     $0, (%r9)           /* Clear dest register */
        movq     $0, %r8             /* Clear tmp register */
        movq     -72(%rbp), %r8      /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -80(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, (%r8)        /* Moving temp to dst */
        movq     $0, -96(%rbp)       /* mov_q    vr19, $4 */
        movq     $0, %r11            /* Clear temp register */
        movq     $4, %r11            /* Moving src to temp */
        movq     %r11, -96(%rbp)     /* Moving temp to dst */
        nop                          /* add_q    vr20, vr19, vr13 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -96(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -48(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -104(%rbp)    /* Moving temp to dest */
        movq     $0, -112(%rbp)      /* mov_l    vr21, $3 */
        movq     $0, %r11            /* Clear temp register */
        movl     $3, %r11d           /* Moving src to temp */
        movl     %r11d, -112(%rbp)   /* Moving temp to dst */
        movq     $0, %rcx            /* mov_l    (vr20), vr21 */
        movq     -104(%rbp), %rcx    /* Move Address to tmp register */
        movq     $0, (%rcx)          /* Clear dest register */
        movq     $0, %rdx            /* Clear tmp register */
        movq     -104(%rbp), %rdx    /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -112(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, (%rdx)       /* Moving temp to dst */
        movq     $0, %rsi            /* add_l    vr28, (vr16), (vr20) */
        movq     -72(%rbp), %rsi     /* Move Address to tmp register */
        movq     $0, %rdi            /* Clear tmp register */
        movq     -104(%rbp), %rdi    /* Move Address to tmp register */
        nop                          /* dst = src1 + src2 */
        movq     $0, %r11            /* Clear tmp register */
        movl     (%rsi), %r11d       /* Moving SRC1 to temp */
        addl     (%rdi), %r11d       /* Applying operation to temp with SRC2 */
        movl     %r11d, -168(%rbp)   /* Moving temp to dest */
        movq     $0, %rax            /* mov_l    vr0, vr28 */
        movq     $0, %r11            /* Clear temp register */
        movl     -168(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, %eax         /* Moving temp to dst */
        popq     %r15                /* leave    $8 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $112, %rsp
        popq     %rbp
        ret                          /* ret       */