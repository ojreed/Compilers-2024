int main(void) {
  int i, n, sum;

  i = 1;
  n = 11;
  sum = 0;

  do {
    sum = sum + i;
    i = i + 1;
  } while (i < n);

  return sum;
}



        .section .text

        .globl main
main:
        pushq    %rbp                /* enter    $0 */
        movq     %rsp, %rbp
        subq     $32, %rsp
        pushq    %rbp                /* Pushing Callee saved to stack */
        pushq    %rbx                /* Pushing Callee saved to stack */
        pushq    %r12                /* Pushing Callee saved to stack */
        pushq    %r13                /* Pushing Callee saved to stack */
        pushq    %r14                /* Pushing Callee saved to stack */
        pushq    %r15                /* Pushing Callee saved to stack */
        movl     $1, %r11d           /* mov_l    vr14, $1 */
        movl     %r11d, -32(%rbp)    /* Moving temp to dst */
        movl     -32(%rbp), %r11d    /* mov_l    vr11, vr14 */
        movl     %r11d, -8(%rbp)     /* Moving temp to dst */
        movl     $11, %r11d          /* mov_l    vr15, $11 */
        movl     %r11d, -40(%rbp)    /* Moving temp to dst */
        movl     -40(%rbp), %r11d    /* mov_l    vr12, vr15 */
        movl     %r11d, -16(%rbp)    /* Moving temp to dst */
        movl     $0, %r11d           /* mov_l    vr16, $0 */
        movl     %r11d, -48(%rbp)    /* Moving temp to dst */
        movl     -48(%rbp), %r11d    /* mov_l    vr13, vr16 */
        movl     %r11d, -24(%rbp)    /* Moving temp to dst */
.L0_do_while_loop:
        nop                          /* add_l    vr17, vr13, vr11 */
        movl     -24(%rbp), %r11d    /* Moving SRC1 to temp */
        addl     -8(%rbp), %r11d     /* Applying operation to temp with SRC2 */
        movl     %r11d, -56(%rbp)    /* Moving temp to dest */
        movl     -56(%rbp), %r11d    /* mov_l    vr13, vr17 */
        movl     %r11d, -24(%rbp)    /* Moving temp to dst */
        movl     $1, %r11d           /* mov_l    vr18, $1 */
        movl     %r11d, -64(%rbp)    /* Moving temp to dst */
        nop                          /* add_l    vr19, vr11, vr18 */
        movl     -8(%rbp), %r11d     /* Moving SRC1 to temp */
        addl     -64(%rbp), %r11d    /* Applying operation to temp with SRC2 */
        movl     %r11d, -72(%rbp)    /* Moving temp to dest */
        movl     -72(%rbp), %r11d    /* mov_l    vr11, vr19 */
        movl     %r11d, -8(%rbp)     /* Moving temp to dst */
        nop                          /* cmplt_l  vr20, vr11, vr12 */
        movl     -8(%rbp), %r11d     /* Moving SRC1 to temp */
        cmpl     -16(%rbp), %r11d    /* Compare SRC1 and SRC2 */
        setl     -80(%rbp)           /* Store Result Flag in DST */
        movq     $0, %r11            /* cjmp_t   vr20, .L0_do_while_loop */
        cmpq     %r11, -80(%rbp)     /* Compare dst with 0 */
        jne      .L0_do_while_loop   /* jumping if to dst if true (dst !=0) */
.L0_end_do_while_loop:
        movl     -24(%rbp), %r11d    /* mov_l    vr0, vr13 */
        movl     %r11d, %eax         /* Moving temp to dst */
.Lmain_return:
        popq     %r15                /* leave    $0 */
        popq     %r14                /* Popping callee saved back to proper register */
        popq     %r13                /* Popping callee saved back to proper register */
        popq     %r12                /* Popping callee saved back to proper register */
        popq     %rbx                /* Popping callee saved back to proper register */
        popq     %rbp                /* Popping callee saved back to proper register */
        addq     $32, %rsp
        popq     %rbp
        ret                          /* ret       */