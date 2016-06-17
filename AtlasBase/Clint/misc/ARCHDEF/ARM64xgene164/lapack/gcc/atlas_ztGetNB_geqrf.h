#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,280,448,504,616,896,1232,2464
 * N : 25,112,168,280,448,504,616,896,1232,2464
 * NB : 2,9,28,18,18,56,56,56,112,112
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 2; \
   else if ((n_) < 140) (nb_) = 9; \
   else if ((n_) < 224) (nb_) = 28; \
   else if ((n_) < 476) (nb_) = 18; \
   else if ((n_) < 1064) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
