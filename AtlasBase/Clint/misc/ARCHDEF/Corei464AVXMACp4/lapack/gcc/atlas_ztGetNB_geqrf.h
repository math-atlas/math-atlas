#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,459,521,645,769,893,1018,1266,1515,2012,4000
 * N : 25,149,211,273,335,397,459,521,645,769,893,1018,1266,1515,2012,4000
 * NB : 12,24,48,48,48,52,52,56,64,72,72,76,120,120,120,240
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 87) (nb_) = 12; \
   else if ((n_) < 180) (nb_) = 24; \
   else if ((n_) < 366) (nb_) = 48; \
   else if ((n_) < 490) (nb_) = 52; \
   else if ((n_) < 583) (nb_) = 56; \
   else if ((n_) < 707) (nb_) = 64; \
   else if ((n_) < 955) (nb_) = 72; \
   else if ((n_) < 1142) (nb_) = 76; \
   else if ((n_) < 3006) (nb_) = 120; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
