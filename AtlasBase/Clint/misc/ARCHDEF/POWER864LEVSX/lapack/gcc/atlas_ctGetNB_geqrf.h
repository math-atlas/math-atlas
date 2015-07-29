#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,397,521,1018,1515,1763,2012,3006,4000
 * N : 25,87,149,211,273,397,521,1018,1515,1763,2012,3006,4000
 * NB : 4,16,24,24,28,28,32,32,32,32,36,60,72
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 118) (nb_) = 16; \
   else if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 459) (nb_) = 28; \
   else if ((n_) < 1887) (nb_) = 32; \
   else if ((n_) < 2509) (nb_) = 36; \
   else if ((n_) < 3503) (nb_) = 60; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
