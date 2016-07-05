#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,240,288,336,720,1056,1248,1296,1344,1440,2880
 * N : 25,144,240,288,336,720,1056,1248,1296,1344,1440,2880
 * NB : 2,12,12,12,48,36,36,36,36,48,48,192
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 312) (nb_) = 12; \
   else if ((n_) < 528) (nb_) = 48; \
   else if ((n_) < 1320) (nb_) = 36; \
   else if ((n_) < 2160) (nb_) = 48; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
