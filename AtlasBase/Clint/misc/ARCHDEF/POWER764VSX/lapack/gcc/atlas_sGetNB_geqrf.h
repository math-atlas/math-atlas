#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1760,3600
 * N : 25,160,240,320,400,880,1760,3600
 * NB : 4,20,16,16,80,80,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 20; \
   else if ((n_) < 360) (nb_) = 16; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
