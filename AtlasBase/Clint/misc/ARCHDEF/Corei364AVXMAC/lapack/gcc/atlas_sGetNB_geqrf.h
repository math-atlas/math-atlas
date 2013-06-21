#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,1120,2320,3520,4080,4400,4720
 * N : 25,240,320,400,560,1120,2320,3520,4080,4400,4720
 * NB : 2,16,20,80,80,80,160,160,160,160,800
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 2; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 360) (nb_) = 20; \
   else if ((n_) < 1720) (nb_) = 80; \
   else if ((n_) < 4560) (nb_) = 160; \
   else (nb_) = 800; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
