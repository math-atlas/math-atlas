#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,480,528,576,720,1008,2064
 * N : 25,96,240,480,528,576,720,1008,2064
 * NB : 2,12,16,16,48,48,48,48,48
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 2; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 504) (nb_) = 16; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
