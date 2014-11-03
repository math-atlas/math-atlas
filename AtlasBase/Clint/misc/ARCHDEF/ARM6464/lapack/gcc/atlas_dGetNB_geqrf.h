#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,140,256,314,372,488,952,1880
 * N : 25,140,256,314,372,488,952,1880
 * NB : 8,10,24,40,40,40,40,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 82) (nb_) = 8; \
   else if ((n_) < 198) (nb_) = 10; \
   else if ((n_) < 285) (nb_) = 24; \
   else if ((n_) < 1416) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
