
        .section .text

        .globl main
main:
        enter    $0
        localaddr vr13, vr11         /* Store pointer to local variable */
        mov_q    vr12, vr13
        mov_l    vr14, $42           /* Initialize literal int */
        mov_q    (vr15), vr12        /* Store Pointed to Value */
        localaddr vr16, vr15         /* Get Address of ref */
        mov_l    (vr16), vr14
        mov_l    vr0, vr11
.Lmain_return:
        leave    $0
        ret      
bash-5.2$ ./nearly_cc input/example10.c 

        .section .text

        .globl main
main:
        pushq    %rbp                /* enter    $0 */
        movq     %rsp, %rbp
        subq     $112, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        leaq     -16(%rbp), %r12     /* localaddr vr13, vr11 */
        movq     %r12, -32(%rbp)     /* Moving temp to dst */
        movq     $0, -24(%rbp)       /* mov_q    vr12, vr13 */
        movq     $0, %r11            /* Clear temp register */
        movq     -32(%rbp), %r11     /* Moving src to temp */
        movq     %r11, -24(%rbp)     /* Moving temp to dst */
        movq     $0, -40(%rbp)       /* mov_l    vr14, $42 */
        movq     $0, %r11            /* Clear temp register */
        movl     $42, %r11d          /* Moving src to temp */
        movl     %r11d, -40(%rbp)    /* Moving temp to dst */
        movq     $0, %r9             /* mov_q    (vr15), vr12 */
        movq     -48(%rbp), %r9      /* Move Address to tmp register */
        movq     $0, (%r9)           /* Clear dest register */
        movq     $0, %r8             /* Clear tmp register */
        movq     -48(%rbp), %r8      /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movq     -24(%rbp), %r11     /* Moving src to temp */
        movq     %r11, (%r8)         /* Moving temp to dst */
        leaq     -48(%rbp), %r12     /* localaddr vr16, vr15 */
        movq     %r12, -56(%rbp)     /* Moving temp to dst */
        movq     $0, %rcx            /* mov_l    (vr16), vr14 */
        movq     -56(%rbp), %rcx     /* Move Address to tmp register */
        movq     $0, (%rcx)          /* Clear dest register */
        movq     $0, %rdx            /* Clear tmp register */
        movq     -56(%rbp), %rdx     /* Move Address to tmp register */
        movq     $0, %r11            /* Clear temp register */
        movl     -40(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, (%rdx)       /* Moving temp to dst */
        movq     $0, %rax            /* mov_l    vr0, vr11 */
        movq     $0, %r11            /* Clear temp register */
        movl     -16(%rbp), %r11d    /* Moving src to temp */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lmain_return:
        popq     %r15                /* leave    $0 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $112, %rsp
        popq     %rbp
        ret                          /* ret       */