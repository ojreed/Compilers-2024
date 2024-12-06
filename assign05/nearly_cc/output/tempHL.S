
        .section .text

        .globl sum
sum:
        pushq    %rbp                /* enter    $0 */
        movq     %rsp, %rbp
        subq     $128, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        movq     $0, -32(%rbp)       /* mov_q    vr13, vr1 */
        movq     $0, %r11            /* Clear temp register */
        movq     %rdi, %r11          /* Moving src to temp */
        movq     %r11, -32(%rbp)     /* Moving temp to dst */
        movq     $0, -40(%rbp)       /* mov_l    vr14, vr2 */
        movq     $0, %r11            /* Clear temp register */
        movl     %esi, %r11d         /* Moving src to temp */
        movl     %r11d, -40(%rbp)    /* Moving temp to dst */
        movq     $0, -48(%rbp)       /* mov_l    vr15, $0 */
        movq     $0, %r11            /* Clear temp register */
        movl     $0, %r11d           /* Moving src to temp */
        movl     %r11d, -48(%rbp)    /* Moving temp to dst */
        movq     $0, -24(%rbp)       /* mov_l    vr12, vr15 */
        movq     $0, %r11            /* Clear temp register */
        movl     -48(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, -24(%rbp)    /* Moving temp to dst */
.L0_for_loop:
        movq     $0, -56(%rbp)       /* mov_l    vr16, $0 */
        movq     $0, %r11            /* Clear temp register */
        movl     $0, %r11d           /* Moving src to temp */
        movl     %r11d, -56(%rbp)    /* Moving temp to dst */
        movq     $0, -16(%rbp)       /* mov_l    vr11, vr16 */
        movq     $0, %r11            /* Clear temp register */
        movl     -56(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, -16(%rbp)    /* Moving temp to dst */
.L0_for_loop_comp:
        nop                          /* cmplt_l  vr17, vr11, vr14 */
        movq     $0, %r11            /* Clear tmp register */
        movl     -16(%rbp), %r11d    /* Moving SRC1 to temp */
        cmpl     -40(%rbp), %r11d    /* Compare SRC1 and SRC2 */
        setl     -64(%rbp)           /* Store Result Flag in DST */
        movq     $0, %r11            /* cjmp_f   vr17, .L0_end_for_loop */
        movb     $0, %r11b           /* Moving dst to temp */
        cmpb     %r11b, -64(%rbp)    /* Compare dst with 0 */
        je       .L0_end_for_loop    /* jumping if to dst if false (dst == 0) */
.L0_for_loop_body:
        movq     $0, -72(%rbp)       /* mov_q    vr18, vr13 */
        movq     $0, %r11            /* Clear temp register */
        movq     -32(%rbp), %r11     /* Moving src to temp */
        movq     %r11, -72(%rbp)     /* Moving temp to dst */
        nop                          /* add_q    vr11, vr11, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -16(%rbp), %r11     /* Moving SRC1 to temp */
        addq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -16(%rbp)     /* Moving temp to dest */
        nop                          /* mul_q    vr19, vr11, $4 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -16(%rbp), %r11     /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -80(%rbp)     /* Moving temp to dest */
        nop                          /* add_q    vr20, vr19, vr18 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -80(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -72(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -88(%rbp)     /* Moving temp to dest */
        nop                          /* sub_q    vr11, vr11, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -16(%rbp), %r11     /* Moving SRC1 to temp */
        subq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -16(%rbp)     /* Moving temp to dest */
        movq     $0, %r9             /* add_l    vr21, vr12, (vr20) */
        movq     -88(%rbp), %r9      /* Move Address to tmp register */
        nop                          /* dst = src1 + src2 */
        movq     $0, %r11            /* Clear tmp register */
        movl     -24(%rbp), %r11d    /* Moving SRC1 to temp */
        addl     (%r9), %r11d        /* Applying operation to temp with SRC2 */
        movl     %r11d, -96(%rbp)    /* Moving temp to dest */
        movq     $0, -24(%rbp)       /* mov_l    vr12, vr21 */
        movq     $0, %r11            /* Clear temp register */
        movl     -96(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, -24(%rbp)    /* Moving temp to dst */
        movq     $0, -104(%rbp)      /* mov_l    vr22, $1 */
        movq     $0, %r11            /* Clear temp register */
        movl     $1, %r11d           /* Moving src to temp */
        movl     %r11d, -104(%rbp)   /* Moving temp to dst */
        nop                          /* add_l    vr23, vr11, vr22 */
        movq     $0, %r11            /* Clear tmp register */
        movl     -16(%rbp), %r11d    /* Moving SRC1 to temp */
        addl     -104(%rbp), %r11d   /* Applying operation to temp with SRC2 */
        movl     %r11d, -112(%rbp)   /* Moving temp to dest */
        movq     $0, -16(%rbp)       /* mov_l    vr11, vr23 */
        movq     $0, %r11            /* Clear temp register */
        movl     -112(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, -16(%rbp)    /* Moving temp to dst */
        jmp      .L0_for_loop_comp   /* jmp      .L0_for_loop_comp */
.L0_end_for_loop:
        movq     $0, %rax            /* mov_l    vr0, vr12 */
        movq     $0, %r11            /* Clear temp register */
        movl     -24(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lsum_return:
        popq     %r15                /* leave    $0 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $128, %rsp
        popq     %rbp
        ret                          /* ret       */

        .globl main
main:
        pushq    %rbp                /* enter    $12 */
        movq     %rsp, %rbp
        subq     $224, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        leaq     -12(%rbp), %r12     /* localaddr vr12, $0 */
        movq     %r12, -40(%rbp)     /* Moving temp to dst */
        movq     $0, -48(%rbp)       /* mov_l    vr13, $0 */
        movq     $0, %r11            /* Clear temp register */
        movl     $0, %r11d           /* Moving src to temp */
        movl     %r11d, -48(%rbp)    /* Moving temp to dst */
        movq     $0, -56(%rbp)       /* mov_q    vr14, vr12 */
        movq     $0, %r11            /* Clear temp register */
        movq     -40(%rbp), %r11     /* Moving src to temp */
        movq     %r11, -56(%rbp)     /* Moving temp to dst */
        nop                          /* add_q    vr13, vr13, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -48(%rbp), %r11     /* Moving SRC1 to temp */
        addq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -48(%rbp)     /* Moving temp to dest */
        nop                          /* mul_q    vr15, vr13, $4 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -48(%rbp), %r11     /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -64(%rbp)     /* Moving temp to dest */
        nop                          /* add_q    vr16, vr15, vr14 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -64(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -56(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -72(%rbp)     /* Moving temp to dest */
        nop                          /* sub_q    vr13, vr13, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -48(%rbp), %r11     /* Moving SRC1 to temp */
        subq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -48(%rbp)     /* Moving temp to dest */
        movq     $0, -80(%rbp)       /* mov_l    vr17, $1 */
        movq     $0, %r11            /* Clear temp register */
        movl     $1, %r11d           /* Moving src to temp */
        movl     %r11d, -80(%rbp)    /* Moving temp to dst */
        movq     $0, %r9             /* mov_l    (vr16), vr17 */
        movq     -72(%rbp), %r9      /* Move Address to tmp register */
        movq     $0, (%r9)           /* Clear dest register */
        movq     $0, %r8             /* Clear tmp register */
        movq     -72(%rbp), %r8      /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -80(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, (%r8)        /* Moving temp to dst */
        leaq     -12(%rbp), %r12     /* localaddr vr18, $0 */
        movq     %r12, -88(%rbp)     /* Moving temp to dst */
        movq     $0, -96(%rbp)       /* mov_l    vr19, $1 */
        movq     $0, %r11            /* Clear temp register */
        movl     $1, %r11d           /* Moving src to temp */
        movl     %r11d, -96(%rbp)    /* Moving temp to dst */
        movq     $0, -104(%rbp)      /* mov_q    vr20, vr18 */
        movq     $0, %r11            /* Clear temp register */
        movq     -88(%rbp), %r11     /* Moving src to temp */
        movq     %r11, -104(%rbp)    /* Moving temp to dst */
        nop                          /* add_q    vr19, vr19, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -96(%rbp), %r11     /* Moving SRC1 to temp */
        addq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -96(%rbp)     /* Moving temp to dest */
        nop                          /* mul_q    vr21, vr19, $4 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -96(%rbp), %r11     /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -112(%rbp)    /* Moving temp to dest */
        nop                          /* add_q    vr22, vr21, vr20 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -112(%rbp), %r11    /* Moving SRC1 to temp */
        addq     -104(%rbp), %r11    /* Applying operation to temp with SRC2 */
        movq     %r11, -120(%rbp)    /* Moving temp to dest */
        nop                          /* sub_q    vr19, vr19, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -96(%rbp), %r11     /* Moving SRC1 to temp */
        subq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -96(%rbp)     /* Moving temp to dest */
        movq     $0, -128(%rbp)      /* mov_l    vr23, $2 */
        movq     $0, %r11            /* Clear temp register */
        movl     $2, %r11d           /* Moving src to temp */
        movl     %r11d, -128(%rbp)   /* Moving temp to dst */
        movq     $0, %rcx            /* mov_l    (vr22), vr23 */
        movq     -120(%rbp), %rcx    /* Move Address to tmp register */
        movq     $0, (%rcx)          /* Clear dest register */
        movq     $0, %rdx            /* Clear tmp register */
        movq     -120(%rbp), %rdx    /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -128(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, (%rdx)       /* Moving temp to dst */
        leaq     -12(%rbp), %r12     /* localaddr vr24, $0 */
        movq     %r12, -136(%rbp)    /* Moving temp to dst */
        movq     $0, -144(%rbp)      /* mov_l    vr25, $2 */
        movq     $0, %r11            /* Clear temp register */
        movl     $2, %r11d           /* Moving src to temp */
        movl     %r11d, -144(%rbp)   /* Moving temp to dst */
        movq     $0, -152(%rbp)      /* mov_q    vr26, vr24 */
        movq     $0, %r11            /* Clear temp register */
        movq     -136(%rbp), %r11    /* Moving src to temp */
        movq     %r11, -152(%rbp)    /* Moving temp to dst */
        nop                          /* add_q    vr25, vr25, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -144(%rbp), %r11    /* Moving SRC1 to temp */
        addq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -144(%rbp)    /* Moving temp to dest */
        nop                          /* mul_q    vr27, vr25, $4 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -144(%rbp), %r11    /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -160(%rbp)    /* Moving temp to dest */
        nop                          /* add_q    vr28, vr27, vr26 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -160(%rbp), %r11    /* Moving SRC1 to temp */
        addq     -152(%rbp), %r11    /* Applying operation to temp with SRC2 */
        movq     %r11, -168(%rbp)    /* Moving temp to dest */
        nop                          /* sub_q    vr25, vr25, $1 */
        movq     $0, %r11            /* Clear tmp register */
        movq     -144(%rbp), %r11    /* Moving SRC1 to temp */
        subq     $1, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -144(%rbp)    /* Moving temp to dest */
        movq     $0, -176(%rbp)      /* mov_l    vr29, $3 */
        movq     $0, %r11            /* Clear temp register */
        movl     $3, %r11d           /* Moving src to temp */
        movl     %r11d, -176(%rbp)   /* Moving temp to dst */
        movq     $0, %rsi            /* mov_l    (vr28), vr29 */
        movq     -168(%rbp), %rsi    /* Move Address to tmp register */
        movq     $0, (%rsi)          /* Clear dest register */
        movq     $0, %rdi            /* Clear tmp register */
        movq     -168(%rbp), %rdi    /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -176(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, (%rdi)       /* Moving temp to dst */
        leaq     -12(%rbp), %r12     /* localaddr vr30, $0 */
        movq     %r12, -184(%rbp)    /* Moving temp to dst */
        movq     $0, -192(%rbp)      /* mov_l    vr31, $3 */
        movq     $0, %r11            /* Clear temp register */
        movl     $3, %r11d           /* Moving src to temp */
        movl     %r11d, -192(%rbp)   /* Moving temp to dst */
        movq     $0, -200(%rbp)      /* mov_q    vr32, vr30 */
        movq     $0, %r11            /* Clear temp register */
        movq     -184(%rbp), %r11    /* Moving src to temp */
        movq     %r11, -200(%rbp)    /* Moving temp to dst */
        movq     $0, %rdi            /* mov_q    vr1, vr32 */
        movq     $0, %r11            /* Clear temp register */
        movq     -200(%rbp), %r11    /* Moving src to temp */
        movq     %r11, %rdi          /* Moving temp to dst */
        movq     $0, -208(%rbp)      /* mov_q    vr33, vr31 */
        movq     $0, %r11            /* Clear temp register */
        movq     -192(%rbp), %r11    /* Moving src to temp */
        movq     %r11, -208(%rbp)    /* Moving temp to dst */
        movq     $0, %rsi            /* mov_q    vr2, vr33 */
        movq     $0, %r11            /* Clear temp register */
        movq     -208(%rbp), %r11    /* Moving src to temp */
        movq     %r11, %rsi          /* Moving temp to dst */
        call     sum                 /* call     sum */
        movq     $0, -216(%rbp)      /* mov_l    vr34, vr0 */
        movq     $0, %r11            /* Clear temp register */
        movl     %eax, %r11d         /* Moving src to temp */
        movl     %r11d, -216(%rbp)   /* Moving temp to dst */
        movq     $0, -32(%rbp)       /* mov_l    vr11, vr34 */
        movq     $0, %r11            /* Clear temp register */
        movl     -216(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, -32(%rbp)    /* Moving temp to dst */
        movq     $0, %rax            /* mov_l    vr0, vr11 */
        movq     $0, %r11            /* Clear temp register */
        movl     -32(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lmain_return:
        popq     %r15                /* leave    $12 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $224, %rsp
        popq     %rbp
        ret                          /* ret       */