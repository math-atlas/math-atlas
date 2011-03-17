#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,880,1760,2160,2640,3520
 * N : 25,160,400,880,1760,2160,2640,3520
 * NB : 4,80,80,80,80,160,160,320
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 1960) (nb_) = 80; \
   else if ((n_) < 3080) (nb_) = 160; \
   else (nb_) = 320; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
