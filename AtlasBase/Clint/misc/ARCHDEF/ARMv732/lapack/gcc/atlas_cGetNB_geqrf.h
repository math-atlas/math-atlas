#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,480,560,640,720,800
 * N : 25,160,400,480,560,640,720,800
 * NB : 3,20,20,20,32,32,40,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 3; \
   else if ((n_) < 520) (nb_) = 20; \
   else if ((n_) < 680) (nb_) = 32; \
   else if ((n_) < 760) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
