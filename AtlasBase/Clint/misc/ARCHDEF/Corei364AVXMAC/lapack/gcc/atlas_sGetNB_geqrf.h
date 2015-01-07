#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,100,175,325,475,626,1228,2432,4840
 * N : 25,100,175,325,475,626,1228,2432,4840
 * NB : 8,8,16,24,24,40,92,140,164
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 137) (nb_) = 8; \
   else if ((n_) < 250) (nb_) = 16; \
   else if ((n_) < 550) (nb_) = 24; \
   else if ((n_) < 927) (nb_) = 40; \
   else if ((n_) < 1830) (nb_) = 92; \
   else if ((n_) < 3636) (nb_) = 140; \
   else (nb_) = 164; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
