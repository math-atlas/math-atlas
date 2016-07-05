#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,224,448,952,1904,3864
 * N : 25,112,168,224,448,952,1904,3864
 * NB : 4,8,56,56,56,56,56,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 2884) (nb_) = 56; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
