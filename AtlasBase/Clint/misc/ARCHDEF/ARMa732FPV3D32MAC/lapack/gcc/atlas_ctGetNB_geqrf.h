#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,288,336,480,576,624,672,720,960,1968
 * N : 25,240,288,336,480,576,624,672,720,960,1968
 * NB : 4,4,28,36,44,16,24,36,48,48,48
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 264) (nb_) = 4; \
   else if ((n_) < 312) (nb_) = 28; \
   else if ((n_) < 408) (nb_) = 36; \
   else if ((n_) < 528) (nb_) = 44; \
   else if ((n_) < 600) (nb_) = 16; \
   else if ((n_) < 648) (nb_) = 24; \
   else if ((n_) < 696) (nb_) = 36; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
