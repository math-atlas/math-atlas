#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,560,720,1520
 * N : 25,160,240,320,400,480,560,720,1520
 * NB : 1,8,24,24,40,40,80,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 1; \
   else if ((n_) < 200) (nb_) = 8; \
   else if ((n_) < 360) (nb_) = 24; \
   else if ((n_) < 520) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
