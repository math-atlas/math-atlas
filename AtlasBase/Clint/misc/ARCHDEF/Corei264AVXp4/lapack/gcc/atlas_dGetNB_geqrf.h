#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,137,193,250,363,476,702,928,1832,3640
 * N : 25,137,193,250,363,476,702,928,1832,3640
 * NB : 12,12,24,24,24,48,48,72,96,148
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 165) (nb_) = 12; \
   else if ((n_) < 419) (nb_) = 24; \
   else if ((n_) < 815) (nb_) = 48; \
   else if ((n_) < 1380) (nb_) = 72; \
   else if ((n_) < 2736) (nb_) = 96; \
   else (nb_) = 148; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
