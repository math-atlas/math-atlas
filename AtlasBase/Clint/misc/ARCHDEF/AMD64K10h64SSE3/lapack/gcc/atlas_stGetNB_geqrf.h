#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,144,216,288,360,720,1512,3024,4536,6120
 * N : 25,144,216,288,360,720,1512,3024,4536,6120
 * NB : 4,12,16,72,72,72,72,144,144,216
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 252) (nb_) = 16; \
   else if ((n_) < 2268) (nb_) = 72; \
   else if ((n_) < 5328) (nb_) = 144; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
