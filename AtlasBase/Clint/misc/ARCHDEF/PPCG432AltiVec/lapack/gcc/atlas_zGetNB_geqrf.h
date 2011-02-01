#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,168,192,360,720
 * N : 25,96,144,168,192,360,720
 * NB : 5,12,12,12,24,24,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 5; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 540) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
