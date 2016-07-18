#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,1040,1520,2080,3120,3680,3920,4240
 * N : 25,240,480,1040,1520,2080,3120,3680,3920,4240
 * NB : 4,80,80,80,80,160,160,160,160,400
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 1800) (nb_) = 80; \
   else if ((n_) < 4080) (nb_) = 160; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
