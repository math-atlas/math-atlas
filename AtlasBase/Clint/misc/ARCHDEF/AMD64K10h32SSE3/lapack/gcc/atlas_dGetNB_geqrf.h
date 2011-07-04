#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,360,420,480,600,1260,2580
 * N : 25,120,300,360,420,480,600,1260,2580
 * NB : 5,12,12,18,24,60,60,60,60
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 5; \
   else if ((n_) < 330) (nb_) = 12; \
   else if ((n_) < 390) (nb_) = 18; \
   else if ((n_) < 450) (nb_) = 24; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
