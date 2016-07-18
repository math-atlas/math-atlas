#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,216,432,900,1116,1368,1836,3708
 * N : 25,108,216,432,900,1116,1368,1836,3708
 * NB : 2,36,36,36,36,36,72,108,108
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 2; \
   else if ((n_) < 1242) (nb_) = 36; \
   else if ((n_) < 1602) (nb_) = 72; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
