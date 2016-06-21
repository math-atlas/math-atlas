#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,720,880,1040,2080
 * N : 25,240,320,400,480,720,880,1040,2080
 * NB : 12,16,16,16,24,24,24,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else if ((n_) < 440) (nb_) = 16; \
   else if ((n_) < 960) (nb_) = 24; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
