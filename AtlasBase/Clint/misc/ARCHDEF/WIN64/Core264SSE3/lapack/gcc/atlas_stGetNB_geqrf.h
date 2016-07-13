#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,216,468,684,936,1908,3816
 * N : 25,108,216,468,684,936,1908,3816
 * NB : 4,40,40,36,36,72,72,72
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 4; \
   else if ((n_) < 342) (nb_) = 40; \
   else if ((n_) < 810) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
