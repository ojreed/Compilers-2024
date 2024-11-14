
        .section .text

        .globl sum
sum:
        pushq    %rbp                /* enter    $0 */
        movq     %rsp, %rbp
        subq     $32, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        movq     %rdi, %r11          /* mov_q    vr13, vr1 */
        movq     %r11, -24(%rbp)     /* Moving temp to dst */
        movl     %esi, %r11d         /* mov_l    vr14, vr2 */
        movl     %r11d, -32(%rbp)    /* Moving temp to dst */
        movl     $0, %r11d           /* mov_l    vr15, $0 */
        movl     %r11d, -40(%rbp)    /* Moving temp to dst */
        movl     -40(%rbp), %r11d    /* mov_l    vr12, vr15 */
        movl     %r11d, -16(%rbp)    /* Moving temp to dst */
.L0_for_loop:
        movl     $0, %r11d           /* mov_l    vr16, $0 */
        movl     %r11d, -48(%rbp)    /* Moving temp to dst */
        movl     -48(%rbp), %r11d    /* mov_l    vr11, vr16 */
        movl     %r11d, -8(%rbp)     /* Moving temp to dst */
.L0_for_loop_comp:
        nop                          /* cmplt_l  vr17, vr11, vr14 */
        movl     -8(%rbp), %r11d     /* Moving SRC1 to temp */
        cmpl     -32(%rbp), %r11d    /* Compare SRC1 and SRC2 */
        setl     -56(%rbp)           /* Store Result Flag in DST */
        movb     $0, %r11b           /* cjmp_f   vr17, .L0_end_for_loop */
        cmpb     %r11b, -56(%rbp)    /* Compare dst with 0 */
        je       .L0_end_for_loop    /* jumping if to dst if false (dst == 0) */
.L0_for_loop_body:
        movq     -24(%rbp), %r11     /* mov_q    vr18, vr13 */
        movq     %r11, -64(%rbp)     /* Moving temp to dst */
        nop                          /* mul_q    vr19, vr11, $4 */
        movq     -8(%rbp), %r11      /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -72(%rbp)     /* Moving temp to dest */
        nop                          /* add_q    vr20, vr19, vr18 */
        movq     -72(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -64(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -80(%rbp)     /* Moving temp to dest */
        movl     -80(%rbp), %r9d     /* add_l    vr21, vr12, (vr20) */
        nop                          /* dst = src1 + src2 */
        movl     -16(%rbp), %r11d    /* Moving SRC1 to temp */
        addl     (%r9), %r11d        /* Applying operation to temp with SRC2 */
        movl     %r11d, -88(%rbp)    /* Moving temp to dest */
        movl     -88(%rbp), %r11d    /* mov_l    vr12, vr21 */
        movl     %r11d, -16(%rbp)    /* Moving temp to dst */
        movl     $1, %r11d           /* mov_l    vr22, $1 */
        movl     %r11d, -96(%rbp)    /* Moving temp to dst */
        nop                          /* add_l    vr23, vr11, vr22 */
        movl     -8(%rbp), %r11d     /* Moving SRC1 to temp */
        addl     -96(%rbp), %r11d    /* Applying operation to temp with SRC2 */
        movl     %r11d, -104(%rbp)   /* Moving temp to dest */
        movl     -104(%rbp), %r11d   /* mov_l    vr11, vr23 */
        movl     %r11d, -8(%rbp)     /* Moving temp to dst */
        jmp      .L0_for_loop_comp   /* jmp      .L0_for_loop_comp */
.L0_end_for_loop:
        movl     -16(%rbp), %r11d    /* mov_l    vr0, vr12 */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lsum_return:
        popq     %r15                /* leave    $0 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $32, %rsp
        popq     %rbp
        ret                          /* ret       */

        .globl main
main:
        pushq    %rbp                /* enter    $12 */
        movq     %rsp, %rbp
        subq     $32, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        leaq     $0, -28(%rbp)       /* localaddr vr12, $0 */
        movl     $0, %r11d           /* mov_l    vr13, $0 */
        movl     %r11d, -36(%rbp)    /* Moving temp to dst */
        movq     -28(%rbp), %r11     /* mov_q    vr14, vr12 */
        movq     %r11, -44(%rbp)     /* Moving temp to dst */
        nop                          /* mul_q    vr15, vr13, $4 */
        movq     -36(%rbp), %r11     /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -52(%rbp)     /* Moving temp to dest */
        nop                          /* add_q    vr16, vr15, vr14 */
        movq     -52(%rbp), %r11     /* Moving SRC1 to temp */
        addq     -44(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -60(%rbp)     /* Moving temp to dest */
        movl     $1, %r11d           /* mov_l    vr17, $1 */
        movl     %r11d, -68(%rbp)    /* Moving temp to dst */
        movl     -60(%rbp), %r9d     /* mov_l    (vr16), vr17 */
        movl     -68(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, (%r9)        /* Moving temp to dst */
        leaq     $0, -76(%rbp)       /* localaddr vr18, $0 */
        movl     $1, %r11d           /* mov_l    vr19, $1 */
        movl     %r11d, -84(%rbp)    /* Moving temp to dst */
        movq     -76(%rbp), %r11     /* mov_q    vr20, vr18 */
        movq     %r11, -92(%rbp)     /* Moving temp to dst */
        nop                          /* mul_q    vr21, vr19, $4 */
        movq     -84(%rbp), %r11     /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -100(%rbp)    /* Moving temp to dest */
        nop                          /* add_q    vr22, vr21, vr20 */
        movq     -100(%rbp), %r11    /* Moving SRC1 to temp */
        addq     -92(%rbp), %r11     /* Applying operation to temp with SRC2 */
        movq     %r11, -108(%rbp)    /* Moving temp to dest */
        movl     $2, %r11d           /* mov_l    vr23, $2 */
        movl     %r11d, -116(%rbp)   /* Moving temp to dst */
        movl     -108(%rbp), %r9d    /* mov_l    (vr22), vr23 */
        movl     -116(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, (%r9)        /* Moving temp to dst */
        leaq     $0, -124(%rbp)      /* localaddr vr24, $0 */
        movl     $2, %r11d           /* mov_l    vr25, $2 */
        movl     %r11d, -132(%rbp)   /* Moving temp to dst */
        movq     -124(%rbp), %r11    /* mov_q    vr26, vr24 */
        movq     %r11, -140(%rbp)    /* Moving temp to dst */
        nop                          /* mul_q    vr27, vr25, $4 */
        movq     -132(%rbp), %r11    /* Moving SRC1 to temp */
        imulq    $4, %r11            /* Applying operation to temp with SRC2 */
        movq     %r11, -148(%rbp)    /* Moving temp to dest */
        nop                          /* add_q    vr28, vr27, vr26 */
        movq     -148(%rbp), %r11    /* Moving SRC1 to temp */
        addq     -140(%rbp), %r11    /* Applying operation to temp with SRC2 */
        movq     %r11, -156(%rbp)    /* Moving temp to dest */
        movl     $3, %r11d           /* mov_l    vr29, $3 */
        movl     %r11d, -164(%rbp)   /* Moving temp to dst */
        movl     -156(%rbp), %r9d    /* mov_l    (vr28), vr29 */
        movl     -164(%rbp), %r11d   /* Moving src to temp */
        movl     %r11d, (%r9)        /* Moving temp to dst */
        leaq     $0, -172(%rbp)      /* localaddr vr30, $0 */
        movl     $3, %r11d           /* mov_l    vr31, $3 */
        movl     %r11d, -180(%rbp)   /* Moving temp to dst */
        movl     -172(%rbp), %r11d   /* mov_l    vr1, vr30 */
        movl     %r11d, %edi         /* Moving temp to dst */
        movl     -180(%rbp), %r11d   /* mov_l    vr2, vr31 */
        movl     %r11d, %esi         /* Moving temp to dst */
        call     sum                 /* call     sum */
        movl     %eax, %r11d         /* mov_l    vr32, vr0 */
        movl     %r11d, -188(%rbp)   /* Moving temp to dst */
        movl     -188(%rbp), %r11d   /* mov_l    vr11, vr32 */
        movl     %r11d, -20(%rbp)    /* Moving temp to dst */
        movl     -20(%rbp), %r11d    /* mov_l    vr0, vr11 */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lmain_return:
        popq     %r15                /* leave    $12 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $32, %rsp
        popq     %rbp
        ret                          /* ret       */