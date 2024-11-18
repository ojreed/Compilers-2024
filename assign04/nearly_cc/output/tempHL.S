
        .section .text

        .globl main
main:
        enter    $0
        localaddr vr13, vr11         /* Store pointer to local variable */
        mov_q    vr12, vr13
        mov_l    vr14, $42           /* Initialize literal int */
        mov_q    (vr15), vr12        /* Store Pointed to Value */
        localaddr vr16, (vr15)       /* Get Address of ref */
        mov_l    (vr16), vr14
        mov_l    vr0, vr11
.Lmain_return:
        leave    $0
        ret    