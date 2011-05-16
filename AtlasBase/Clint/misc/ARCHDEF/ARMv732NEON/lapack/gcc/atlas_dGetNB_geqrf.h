#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,640,720,800,1120
 * N : 25,240,320,400,560,640,720,800,1120
 * NB : 1,8,16,16,32,40,80,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 1; \
   else if ((n_) < 280) (nb_) = 8; \
   else if ((n_) < 480) (nb_) = 16; \
   else if ((n_) < 600) (nb_) = 32; \
   else if ((n_) < 680) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
