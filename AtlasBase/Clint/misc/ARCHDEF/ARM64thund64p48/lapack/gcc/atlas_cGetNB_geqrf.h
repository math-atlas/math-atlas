#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,120,168,216,312,408,504,600,696,792,1176,1368,1560
 * N : 25,72,120,168,216,312,408,504,600,696,792,1176,1368,1560
 * NB : 1,11,15,12,31,32,35,35,36,36,43,42,42,51
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 1; \
   else if ((n_) < 96) (nb_) = 11; \
   else if ((n_) < 144) (nb_) = 15; \
   else if ((n_) < 192) (nb_) = 12; \
   else if ((n_) < 264) (nb_) = 31; \
   else if ((n_) < 360) (nb_) = 32; \
   else if ((n_) < 552) (nb_) = 35; \
   else if ((n_) < 744) (nb_) = 36; \
   else if ((n_) < 984) (nb_) = 43; \
   else if ((n_) < 1464) (nb_) = 42; \
   else (nb_) = 51; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
