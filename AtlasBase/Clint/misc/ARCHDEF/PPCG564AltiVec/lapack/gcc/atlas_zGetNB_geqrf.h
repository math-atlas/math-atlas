#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,288,384,768,1536
 * N : 25,96,144,192,240,288,384,768,1536
 * NB : 4,12,12,24,48,48,48,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 216) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
