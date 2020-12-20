00000   mov   r12, 61
00001   mov   r13, 36
00002   lbl   512
00003   mov   r01, 2
00004   io    r00, 11, r01
00005   io    r01, 8, r00
00006   cmpul r01, 40
00007 + jup   512
00008   mov   r50, 62
00009   io    r08, 9, r00
00010   io    r09, 9, r00
00011   io    r10, 9, r00
00012   io    r11, 9, r00
00013   mov   r02, 14
00014   lbl   513
00015   io    r01, 9, r00
00016   st    [r02+0], r01
00017   addi  r02, r02, 1
00018   cmpul r02, 50
00019 + jup   513
00020   mov   r02, 8
00021   lbl   513
00022   ld    r01, [r02+0]
00023   io    r00, 2, r01
00024   addi  r02, r02, 1
00025   cmpul r02, 51
00026 + jup   513
00027   cmpeq r00, r00
00028 + cmpeq r24, 36
00029 + cmpeq r21, 36
00030 + cmpeq r40, 10
00031 + cmpeq r45, 21
00032 + cmpeq r11, 28
00033 + cmpeq r27, 29
00034 + cmpeq r23, 14
00035 + cmpeq r39, 22
00036 + cmpeq r34, 10
00037 + cmpeq r29, 14
00038 + cmpeq r14, 18
00039 + cmpeq r35, 23
00040 + cmpeq r43, 21
00041 + cmpeq r09, 22
00042 + cmpeq r08, 34
00043 + cmpeq r17, 18
00044 + cmpeq r18, 16
00045 + cmpeq r36, 36
00046 + cmpeq r47, 21
00047 + cmpeq r20, 29
00048 + cmpeq r44, 24
00049 + cmpeq r16, 22
00050 + cmpeq r15, 36
00051 + cmpeq r22, 11
00052 + cmpeq r38, 38
00053 + cmpeq r48, 24
00054 + cmpeq r26, 14
00055 + cmpeq r49, 21
00056 + cmpeq r37, 33
00057 + cmpeq r46, 24
00058 + cmpeq r32, 29
00059 + cmpeq r25, 11
00060 + cmpeq r41, 28
00061 + cmpeq r19, 17
00062 + cmpeq r33, 17
00063 + cmpeq r31, 36
00064 + cmpeq r28, 29
00065 + cmpeq r10, 10
00066 + cmpeq r42, 36
00067 + cmpeq r30, 27
00068 + jdn   4095
00069   mov   r01, 1
00070   io    r00, 11, r01
00071   lbl   513
00072   mov   r01, 33
00073   io    r00, 2, r01
00074   mov   r01, 2
00075   io    r01, 11, r01
00076   cmpeq r01, 1
00077 + jdn   513
00078   mov   r02, 8
00079   lbl   514
00080   ld    r01, [r02+0]
00081   io    r00, 10, r01
00082   addi  r02, r02, 1
00083   cmpul r02, 51
00084 + jup   514
00085   jup   513
00086   lbl   513
00087   jdn   512
00088   lbl   4095
00089   mov   r08, 33
00090   mov   r09, 38
00091   mov   r10, 22
00092   mov   r11, 10
00093   mov   r12, 28
00094   mov   r13, 48
