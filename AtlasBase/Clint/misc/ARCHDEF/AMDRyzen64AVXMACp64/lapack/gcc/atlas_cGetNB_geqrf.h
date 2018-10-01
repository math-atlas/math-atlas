#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,205,386,431,476,521,567,612,657,702,748,793,838,883,929,1019,1064,1110,1291,1472,2196,2920
 * N : 25,205,386,431,476,521,567,612,657,702,748,793,838,883,929,1019,1064,1110,1291,1472,2196,2920
 * NB : 1,1,7,10,11,12,15,15,16,16,18,42,46,48,50,48,52,66,52,99,112,192
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 295) (nb_) = 1; \
   else if ((n_) < 408) (nb_) = 7; \
   else if ((n_) < 453) (nb_) = 10; \
   else if ((n_) < 498) (nb_) = 11; \
   else if ((n_) < 544) (nb_) = 12; \
   else if ((n_) < 634) (nb_) = 15; \
   else if ((n_) < 725) (nb_) = 16; \
   else if ((n_) < 770) (nb_) = 18; \
   else if ((n_) < 815) (nb_) = 42; \
   else if ((n_) < 860) (nb_) = 46; \
   else if ((n_) < 906) (nb_) = 48; \
   else if ((n_) < 974) (nb_) = 50; \
   else if ((n_) < 1041) (nb_) = 48; \
   else if ((n_) < 1087) (nb_) = 52; \
   else if ((n_) < 1200) (nb_) = 66; \
   else if ((n_) < 1381) (nb_) = 52; \
   else if ((n_) < 1834) (nb_) = 99; \
   else if ((n_) < 2558) (nb_) = 112; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
