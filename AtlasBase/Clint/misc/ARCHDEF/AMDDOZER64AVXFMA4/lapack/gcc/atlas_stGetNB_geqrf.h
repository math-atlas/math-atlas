#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,360,720,1440,2952,5976
 * N : 25,144,360,720,1440,2952,5976
 * NB : 5,72,72,72,72,72,144
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 5; \
   else if ((n_) < 4464) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
