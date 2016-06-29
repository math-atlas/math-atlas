#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,240,288,384,768,1152,1536,3120
 * N : 25,96,192,240,288,384,768,1152,1536,3120
 * NB : 2,12,4,4,40,44,24,48,48,48
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 2; \
   else if ((n_) < 144) (nb_) = 12; \
   else if ((n_) < 264) (nb_) = 4; \
   else if ((n_) < 336) (nb_) = 40; \
   else if ((n_) < 576) (nb_) = 44; \
   else if ((n_) < 960) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
