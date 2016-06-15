#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,360,720,1080,1440
 * N : 25,360,720,1080,1440
 * NB : 12,12,32,32,48
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 540) (nb_) = 12; \
   else if ((n_) < 1260) (nb_) = 32; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
