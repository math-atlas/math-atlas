#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,640,1280,1920,2240,2640,5280
 * N : 25,160,240,320,640,1280,1920,2240,2640,5280
 * NB : 4,20,40,32,80,80,80,80,200,224
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 20; \
   else if ((n_) < 280) (nb_) = 40; \
   else if ((n_) < 480) (nb_) = 32; \
   else if ((n_) < 2440) (nb_) = 80; \
   else if ((n_) < 3960) (nb_) = 200; \
   else (nb_) = 224; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
