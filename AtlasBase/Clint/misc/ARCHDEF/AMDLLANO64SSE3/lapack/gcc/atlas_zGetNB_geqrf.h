#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,77,130,183,236,342,448,872,1720
 * N : 25,77,130,183,236,342,448,872,1720
 * NB : 6,12,12,18,18,24,36,48,66
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 51) (nb_) = 6; \
   else if ((n_) < 156) (nb_) = 12; \
   else if ((n_) < 289) (nb_) = 18; \
   else if ((n_) < 395) (nb_) = 24; \
   else if ((n_) < 660) (nb_) = 36; \
   else if ((n_) < 1296) (nb_) = 48; \
   else (nb_) = 66; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
