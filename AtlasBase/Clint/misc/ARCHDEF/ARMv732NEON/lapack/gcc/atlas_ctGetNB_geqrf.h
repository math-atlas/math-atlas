#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,640,720,800,1120
 * N : 25,240,560,640,720,800,1120
 * NB : 4,32,28,36,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 400) (nb_) = 32; \
   else if ((n_) < 600) (nb_) = 28; \
   else if ((n_) < 680) (nb_) = 36; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
