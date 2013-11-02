#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,135,245,300,355,466,908,963,1018,1129,1239,1350,1571,1792,2676,3560
 * N : 25,80,135,245,300,355,466,908,963,1018,1129,1239,1350,1571,1792,2676,3560
 * NB : 12,36,36,36,36,44,44,52,60,60,60,68,72,72,96,96,120
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 12; \
   else if ((n_) < 327) (nb_) = 36; \
   else if ((n_) < 687) (nb_) = 44; \
   else if ((n_) < 935) (nb_) = 52; \
   else if ((n_) < 1184) (nb_) = 60; \
   else if ((n_) < 1294) (nb_) = 68; \
   else if ((n_) < 1681) (nb_) = 72; \
   else if ((n_) < 3118) (nb_) = 96; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
