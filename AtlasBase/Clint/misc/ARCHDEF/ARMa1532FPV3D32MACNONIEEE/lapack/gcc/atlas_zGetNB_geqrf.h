#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,150,300,450,500,550,600,1200
 * N : 25,150,300,450,500,550,600,1200
 * NB : 12,20,20,20,30,50,50,50
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 87) (nb_) = 12; \
   else if ((n_) < 475) (nb_) = 20; \
   else if ((n_) < 525) (nb_) = 30; \
   else (nb_) = 50; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
