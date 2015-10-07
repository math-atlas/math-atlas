#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,420,540,600,720,1440
 * N : 25,180,360,420,540,600,720,1440
 * NB : 2,8,16,20,20,60,60,60
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 2; \
   else if ((n_) < 270) (nb_) = 8; \
   else if ((n_) < 390) (nb_) = 16; \
   else if ((n_) < 570) (nb_) = 20; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
