#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,400,560,640,800,1600,3200
 * N : 25,160,240,400,560,640,800,1600,3200
 * NB : 2,2,40,40,40,40,80,80,80
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 200) (nb_) = 2; \
   else if ((n_) < 720) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
