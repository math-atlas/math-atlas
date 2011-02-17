#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,252,588,1260,2604,3948,4284,4452,4536,4620,4956,5124,5208,5292
 * N : 25,252,588,1260,2604,3948,4284,4452,4536,4620,4956,5124,5208,5292
 * NB : 5,84,84,84,84,84,84,56,168,168,168,140,252,840
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 138) (nb_) = 5; \
   else if ((n_) < 4368) (nb_) = 84; \
   else if ((n_) < 4494) (nb_) = 56; \
   else if ((n_) < 5040) (nb_) = 168; \
   else if ((n_) < 5166) (nb_) = 140; \
   else if ((n_) < 5250) (nb_) = 252; \
   else (nb_) = 840; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
