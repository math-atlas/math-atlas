#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,150,350,450,500,550,750,1500,2250,2600,3000
 * N : 25,150,350,450,500,550,750,1500,2250,2600,3000
 * NB : 2,20,20,20,50,50,50,100,150,200,200
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 87) (nb_) = 2; \
   else if ((n_) < 475) (nb_) = 20; \
   else if ((n_) < 1125) (nb_) = 50; \
   else if ((n_) < 1875) (nb_) = 100; \
   else if ((n_) < 2425) (nb_) = 150; \
   else (nb_) = 200; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
