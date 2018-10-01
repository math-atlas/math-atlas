#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,120,168,216,312,360,408,600,696,792,984,1176,1272,1368,1464,1560
 * N : 25,72,120,168,216,312,360,408,600,696,792,984,1176,1272,1368,1464,1560
 * NB : 1,11,15,12,27,24,36,43,42,36,51,51,52,52,54,54,59
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 1; \
   else if ((n_) < 96) (nb_) = 11; \
   else if ((n_) < 144) (nb_) = 15; \
   else if ((n_) < 192) (nb_) = 12; \
   else if ((n_) < 264) (nb_) = 27; \
   else if ((n_) < 336) (nb_) = 24; \
   else if ((n_) < 384) (nb_) = 36; \
   else if ((n_) < 504) (nb_) = 43; \
   else if ((n_) < 648) (nb_) = 42; \
   else if ((n_) < 744) (nb_) = 36; \
   else if ((n_) < 1080) (nb_) = 51; \
   else if ((n_) < 1320) (nb_) = 52; \
   else if ((n_) < 1512) (nb_) = 54; \
   else (nb_) = 59; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
