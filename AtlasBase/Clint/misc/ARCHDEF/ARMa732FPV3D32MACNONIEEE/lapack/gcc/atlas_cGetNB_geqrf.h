#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,288,624,912,1248
 * N : 25,288,624,912,1248
 * NB : 12,8,28,48,48
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 156) (nb_) = 12; \
   else if ((n_) < 456) (nb_) = 8; \
   else if ((n_) < 768) (nb_) = 28; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
