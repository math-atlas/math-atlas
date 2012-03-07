#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,360,792,1152,1368,1440,1512,1584
 * N : 25,144,216,360,792,1152,1368,1440,1512,1584
 * NB : 2,12,72,72,36,36,48,48,40,72
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 576) (nb_) = 72; \
   else if ((n_) < 1260) (nb_) = 36; \
   else if ((n_) < 1476) (nb_) = 48; \
   else if ((n_) < 1548) (nb_) = 40; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
