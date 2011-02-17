#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,360,480,600,720,960
 * N : 25,240,360,480,600,720,960
 * NB : 4,16,16,24,120,120,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 420) (nb_) = 16; \
   else if ((n_) < 540) (nb_) = 24; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
