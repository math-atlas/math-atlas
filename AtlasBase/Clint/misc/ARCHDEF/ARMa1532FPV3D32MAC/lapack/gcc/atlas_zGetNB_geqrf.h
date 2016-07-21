#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,160,240,280,320,640,1280
 * N : 25,80,160,240,280,320,640,1280
 * NB : 4,8,20,20,40,40,40,40
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 4; \
   else if ((n_) < 120) (nb_) = 8; \
   else if ((n_) < 260) (nb_) = 20; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
