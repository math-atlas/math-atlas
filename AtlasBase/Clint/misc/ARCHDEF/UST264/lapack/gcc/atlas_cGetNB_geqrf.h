#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,100,120,140,160,180,340,680
 * N : 25,100,120,140,160,180,340,680
 * NB : 9,8,12,12,20,20,20,20
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 62) (nb_) = 9; \
   else if ((n_) < 110) (nb_) = 8; \
   else if ((n_) < 150) (nb_) = 12; \
   else (nb_) = 20; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
