#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,432,480,528,624,720,768,864
 * N : 25,96,192,432,480,528,624,720,768,864
 * NB : 4,18,18,18,24,24,24,30,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 456) (nb_) = 18; \
   else if ((n_) < 672) (nb_) = 24; \
   else if ((n_) < 744) (nb_) = 30; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
