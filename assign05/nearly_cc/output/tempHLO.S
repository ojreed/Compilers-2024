
        .section .text

        .globl main
main:
        enter    $8
        localaddr vr12, $0           /* Store stack memory in a VReg */
        localaddr vr13, vr12         /* Store pointer to local variable */
        mov_q    vr11, vr13
        mov_q    vr14, vr11          /* Store Struct Address */
        mov_q    vr15, $0            /* Store Member offset in VReg */
        add_q    vr16, vr15, vr14    /* Compute struct member address from struct_base+computed_offset */
        mov_l    vr17, $2            /* Initialize literal int */
        mov_l    (vr16), vr17
        mov_q    vr18, vr11          /* Store Struct Address */
        mov_q    vr19, $4            /* Store Member offset in VReg */
        add_q    vr20, vr19, vr18    /* Compute struct member address from struct_base+computed_offset */
        mov_l    vr21, $3            /* Initialize literal int */
        mov_l    (vr20), vr21
        mov_q    vr22, vr11          /* Store Struct Address */
        mov_q    vr23, $0            /* Store Member offset in VReg */
        add_q    vr24, vr23, vr22    /* Compute struct member address from struct_base+computed_offset */
        mov_q    vr25, vr11          /* Store Struct Address */
        mov_q    vr26, $4            /* Store Member offset in VReg */
        add_q    vr27, vr26, vr25    /* Compute struct member address from struct_base+computed_offset */
        add_l    vr28, (vr24), (vr27) /* Execute the "+" Binary Operation */
        mov_l    vr0, vr28
.Lmain_return:
        leave    $8
        ret     