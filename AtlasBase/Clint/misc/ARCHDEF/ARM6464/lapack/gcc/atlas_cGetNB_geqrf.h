#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,176,328,632,1240
 * N : 25,176,328,632,1240
 * NB : 12,12,24,48,48
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 480) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
