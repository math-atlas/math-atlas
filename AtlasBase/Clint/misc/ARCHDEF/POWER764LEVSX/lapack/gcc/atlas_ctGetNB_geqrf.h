#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,640,720,800,960,1040,1120,2240,4560,9120
 * N : 25,240,560,640,720,800,960,1040,1120,2240,4560,9120
 * NB : 10,24,20,20,28,28,16,16,136,112,152,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 10; \
   else if ((n_) < 400) (nb_) = 24; \
   else if ((n_) < 680) (nb_) = 20; \
   else if ((n_) < 880) (nb_) = 28; \
   else if ((n_) < 1080) (nb_) = 16; \
   else if ((n_) < 1680) (nb_) = 136; \
   else if ((n_) < 3400) (nb_) = 112; \
   else if ((n_) < 6840) (nb_) = 152; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
