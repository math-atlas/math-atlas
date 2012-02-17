#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,560,1120
 * N : 25,240,320,400,480,560,1120
 * NB : 1,12,16,20,32,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 1; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 16; \
   else if ((n_) < 440) (nb_) = 20; \
   else if ((n_) < 520) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
