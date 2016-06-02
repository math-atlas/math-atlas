#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,360,720,1440,2880
 * N : 25,360,720,1440,2880
 * NB : 12,72,32,36,60
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 192) (nb_) = 12; \
   else if ((n_) < 540) (nb_) = 72; \
   else if ((n_) < 1080) (nb_) = 32; \
   else if ((n_) < 2160) (nb_) = 36; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
