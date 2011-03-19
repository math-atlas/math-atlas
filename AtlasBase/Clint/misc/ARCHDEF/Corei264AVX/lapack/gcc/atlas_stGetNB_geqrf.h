#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,800,1680,3440,3520,3600,3840,4320,5200,6080,6960
 * N : 25,160,400,800,1680,3440,3520,3600,3840,4320,5200,6080,6960
 * NB : 4,80,80,80,80,56,80,120,88,80,80,80,88
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 2560) (nb_) = 80; \
   else if ((n_) < 3480) (nb_) = 56; \
   else if ((n_) < 3560) (nb_) = 80; \
   else if ((n_) < 3720) (nb_) = 120; \
   else if ((n_) < 4080) (nb_) = 88; \
   else if ((n_) < 6520) (nb_) = 80; \
   else (nb_) = 88; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
