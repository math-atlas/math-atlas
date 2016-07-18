#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,432,624,720,768,816,864,1728,3504
 * N : 25,96,144,192,432,624,720,768,816,864,1728,3504
 * NB : 2,8,48,48,48,48,48,48,48,96,96,96
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 2; \
   else if ((n_) < 120) (nb_) = 8; \
   else if ((n_) < 840) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
