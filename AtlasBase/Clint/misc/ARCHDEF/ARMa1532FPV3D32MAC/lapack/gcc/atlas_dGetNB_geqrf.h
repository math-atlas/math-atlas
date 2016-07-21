#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,280,360,400,480,1000,2040
 * N : 25,120,240,280,360,400,480,1000,2040
 * NB : 2,8,12,20,20,20,40,40,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 2; \
   else if ((n_) < 180) (nb_) = 8; \
   else if ((n_) < 260) (nb_) = 12; \
   else if ((n_) < 440) (nb_) = 20; \
   else if ((n_) < 1520) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
