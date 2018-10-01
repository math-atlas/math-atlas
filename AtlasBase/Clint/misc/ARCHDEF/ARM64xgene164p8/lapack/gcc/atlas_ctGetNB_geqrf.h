#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,130,235,340,446,657,868,1290,1712,2556,3400
 * N : 25,130,235,340,446,657,868,1290,1712,2556,3400
 * NB : 1,1,3,3,4,4,7,7,8,40,402
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 182) (nb_) = 1; \
   else if ((n_) < 393) (nb_) = 3; \
   else if ((n_) < 762) (nb_) = 4; \
   else if ((n_) < 1501) (nb_) = 7; \
   else if ((n_) < 2134) (nb_) = 8; \
   else if ((n_) < 2978) (nb_) = 40; \
   else (nb_) = 402; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
