#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,100,200,450,550,700,950,1900
 * N : 25,100,200,450,550,700,950,1900
 * NB : 12,50,16,20,50,50,50,50
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 62) (nb_) = 12; \
   else if ((n_) < 150) (nb_) = 50; \
   else if ((n_) < 325) (nb_) = 16; \
   else if ((n_) < 500) (nb_) = 20; \
   else (nb_) = 50; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
