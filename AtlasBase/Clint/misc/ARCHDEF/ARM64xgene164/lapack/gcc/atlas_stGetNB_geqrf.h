#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,400,480,560,1120,2320,3440,4000,4640
 * N : 25,240,400,480,560,1120,2320,3440,4000,4640
 * NB : 1,40,32,32,80,80,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 1; \
   else if ((n_) < 320) (nb_) = 40; \
   else if ((n_) < 520) (nb_) = 32; \
   else if ((n_) < 4320) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
