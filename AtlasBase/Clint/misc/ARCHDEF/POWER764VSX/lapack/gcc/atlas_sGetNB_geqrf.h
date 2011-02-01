#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,560,720,960,2000,3040,3520,3760,3920,4000,4080
 * N : 25,240,480,560,720,960,2000,3040,3520,3760,3920,4000,4080
 * NB : 12,80,20,80,80,80,80,80,80,80,80,80,320
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 80; \
   else if ((n_) < 520) (nb_) = 20; \
   else if ((n_) < 4040) (nb_) = 80; \
   else (nb_) = 320; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
