#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,4000
 * N : 25,87,149,211,273,521,1018,2012,4000
 * NB : 8,24,32,36,40,68,68,68,96
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 118) (nb_) = 24; \
   else if ((n_) < 180) (nb_) = 32; \
   else if ((n_) < 242) (nb_) = 36; \
   else if ((n_) < 397) (nb_) = 40; \
   else if ((n_) < 3006) (nb_) = 68; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
