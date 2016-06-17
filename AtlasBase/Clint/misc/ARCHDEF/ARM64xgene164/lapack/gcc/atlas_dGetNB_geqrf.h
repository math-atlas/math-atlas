#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,480,960,1968
 * N : 25,96,240,480,960,1968
 * NB : 12,48,20,16,48,48
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 12; \
   else if ((n_) < 168) (nb_) = 48; \
   else if ((n_) < 360) (nb_) = 20; \
   else if ((n_) < 720) (nb_) = 16; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
