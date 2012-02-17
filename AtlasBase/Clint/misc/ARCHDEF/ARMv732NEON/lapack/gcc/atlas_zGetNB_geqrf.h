#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,288,360,504,720
 * N : 25,144,216,288,360,504,720
 * NB : 4,12,16,20,36,72,72
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 252) (nb_) = 16; \
   else if ((n_) < 324) (nb_) = 20; \
   else if ((n_) < 432) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
