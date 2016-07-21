#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,280,320,360,480,1000,2040
 * N : 25,120,240,280,320,360,480,1000,2040
 * NB : 4,20,16,16,40,40,40,40,40
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 20; \
   else if ((n_) < 300) (nb_) = 16; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
