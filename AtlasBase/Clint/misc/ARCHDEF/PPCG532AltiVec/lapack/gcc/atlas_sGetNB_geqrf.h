#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,720,1440,1760,2160,2960
 * N : 25,160,240,320,400,480,720,1440,1760,2160,2960
 * NB : 4,8,20,20,80,80,80,80,80,160,160
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 8; \
   else if ((n_) < 360) (nb_) = 20; \
   else if ((n_) < 1960) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
