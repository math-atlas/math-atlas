ROUTINE ATL_USERMM;
   PARAMS :: nmus, nnus, K, pA, pB, pC, pAn, pBn, pCn;
   INT :: nmus, nnus, K;
   DOUBLE_PTR :: pA, pB, pC, pAn, pBn, pCn;

ROUT_LOCALS 
   INT :: i, j, k, K1, incAm;
   VDOUBLE(4) :: VrA0, VrA1, VrA2, VrB;
   VDOUBLE(4) :: VrC00, VrC10, VrC20, VrC01, VrC11, VrC21, VrC02, VrC12, VrC22, VrC03,VrC13, VrC23; 
   DOUBLE_PTR :: pB0;
ROUT_BEGIN
   incAm = K * 12;
   pB0 = pB;
   K = K - 1;
   i = nmus;
   MLOOP:
      j = nnus;
      NLOOP:
// 1st iter peeled to handle init of rC
         VrB = _VBROADCAST(pB[0]);
         VrA0 = pA[0];
         VrC00 = VrA0 * VrB;
         VrA1 = pA[4];
         VrC10 = VrA1 * VrB;
         VrA2 = pA[8];
         VrC20 = VrA2 * VrB;
         pA += 12; 
         
         VrB = _VBROADCAST(pB[1]);
         VrC01 = VrA0 * VrB;
         VrC11 = VrA1 * VrB;
         VrC21 = VrA2 * VrB;
         
         VrB = _VBROADCAST(pB[2]);
         VrC02 = VrA0 * VrB;
         VrC12 = VrA1 * VrB;
         VrC22 = VrA2 * VrB;

         VrB = _VBROADCAST(pB[3]);
         VrC03 = VrA0 * VrB;
         VrC13 = VrA1 * VrB;
         VrC23 = VrA2 * VrB;
         
         pB += 4;

         LOOP k = 0, K
         LOOP_BODY
            
            VrB = _VBROADCAST(pB[0]);
            VrA0 = pA[0];
            VrC00 += VrA0 * VrB;
            
            VrA1 = pA[4];
            VrC10 += VrA1 * VrB;
            
            VrA2 = pA[8];
            VrC20 += VrA2 * VrB;

            VrB = _VBROADCAST(pB[1]);
            VrC01 += VrA0 * VrB;
            VrC11 += VrA1 * VrB;
            VrC21 += VrA2 * VrB;
            
            VrB = _VBROADCAST(pB[2]);
            VrC02 += VrA0 * VrB;
            VrC12 += VrA1 * VrB;
            VrC22 += VrA2 * VrB;

            VrB = _VBROADCAST(pB[3]);
            VrC03 += VrA0 * VrB;
            VrC13 += VrA1 * VrB;
            VrC23 += VrA2 * VrB;
 
            pA += 12;
            pB += 4;
         LOOP_END
         
         pA = pA - incAm;

         pC[0] = VrC00;
         pC[4] = VrC10;
         pC[8] = VrC20;

         pC[12] = VrC01;
         pC[16] = VrC11;
         pC[20] = VrC21;
         
         pC[24] = VrC02;
         pC[28] = VrC12;
         pC[32] = VrC22;
         
         pC[36] = VrC03;
         pC[40] = VrC13;
         pC[44] = VrC23;

         pC += 48; 
         j = j - 1;   
      IF (j > 0) GOTO NLOOP;
      pA += incAm;
      pB = pB0;
      i = i - 1;   
   IF (i > 0) GOTO MLOOP;
   
