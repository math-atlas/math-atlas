#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,204,408,884,1836,3740
 * N : 25,204,408,884,1836,3740
 * NB : 4,68,68,68,68,136
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 114) (nb_) = 4; \
   else if ((n_) < 2788) (nb_) = 68; \
   else (nb_) = 136; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
