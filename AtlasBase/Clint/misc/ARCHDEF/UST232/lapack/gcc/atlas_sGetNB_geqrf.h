#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,252,288,324,360,396,540,1080
 * N : 25,108,252,288,324,360,396,540,1080
 * NB : 4,8,8,12,12,36,36,36,36
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 4; \
   else if ((n_) < 270) (nb_) = 8; \
   else if ((n_) < 342) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
