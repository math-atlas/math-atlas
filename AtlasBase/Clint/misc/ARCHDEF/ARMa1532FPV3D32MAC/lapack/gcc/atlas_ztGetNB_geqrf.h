#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,100,200,450,500,550,700,950,1150,1400,1900
 * N : 25,100,200,450,500,550,700,950,1150,1400,1900
 * NB : 4,20,20,20,50,50,50,50,50,100,100
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 62) (nb_) = 4; \
   else if ((n_) < 475) (nb_) = 20; \
   else if ((n_) < 1275) (nb_) = 50; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
