#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,480,960
 * N : 25,240,320,480,960
 * NB : 4,20,80,80,80
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 20; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
