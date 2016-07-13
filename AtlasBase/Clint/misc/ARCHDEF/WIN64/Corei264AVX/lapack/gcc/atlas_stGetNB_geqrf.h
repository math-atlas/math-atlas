#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,864,1728,3456,6984
 * N : 25,216,432,864,1728,3456,6984
 * NB : 9,72,72,72,72,144,216
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 9; \
   else if ((n_) < 2592) (nb_) = 72; \
   else if ((n_) < 5220) (nb_) = 144; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
