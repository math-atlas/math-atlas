#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,560,800,1600
 * N : 25,160,240,320,400,560,800,1600
 * NB : 2,8,16,16,32,80,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 200) (nb_) = 8; \
   else if ((n_) < 360) (nb_) = 16; \
   else if ((n_) < 480) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
