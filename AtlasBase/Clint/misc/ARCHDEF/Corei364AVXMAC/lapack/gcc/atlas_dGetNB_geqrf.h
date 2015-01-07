#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,85,145,205,265,385,506,747,988,1470,1952,3880
 * N : 25,85,145,205,265,385,506,747,988,1470,1952,3880
 * NB : 8,12,12,24,24,40,40,44,92,116,136,152
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 8; \
   else if ((n_) < 175) (nb_) = 12; \
   else if ((n_) < 325) (nb_) = 24; \
   else if ((n_) < 626) (nb_) = 40; \
   else if ((n_) < 867) (nb_) = 44; \
   else if ((n_) < 1229) (nb_) = 92; \
   else if ((n_) < 1711) (nb_) = 116; \
   else if ((n_) < 2916) (nb_) = 136; \
   else (nb_) = 152; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
