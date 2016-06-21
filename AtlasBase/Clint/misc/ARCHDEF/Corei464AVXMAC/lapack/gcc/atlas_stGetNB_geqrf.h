#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1120,2240,4480,8960
 * N : 25,240,560,1120,2240,4480,8960
 * NB : 2,80,80,80,80,160,320
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 2; \
   else if ((n_) < 3360) (nb_) = 80; \
   else if ((n_) < 6720) (nb_) = 160; \
   else (nb_) = 320; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
