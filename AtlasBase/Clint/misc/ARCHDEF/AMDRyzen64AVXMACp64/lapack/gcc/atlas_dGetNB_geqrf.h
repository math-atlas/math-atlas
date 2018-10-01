#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,135,190,245,300,355,410,466,576,687,797,908,1129,1350,1571,1792,2234,2676,3118,3560
 * N : 25,80,135,190,245,300,355,410,466,576,687,797,908,1129,1350,1571,1792,2234,2676,3118,3560
 * NB : 1,1,6,14,15,15,16,16,19,19,20,20,23,23,24,24,27,27,28,28,30
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 107) (nb_) = 1; \
   else if ((n_) < 162) (nb_) = 6; \
   else if ((n_) < 217) (nb_) = 14; \
   else if ((n_) < 327) (nb_) = 15; \
   else if ((n_) < 438) (nb_) = 16; \
   else if ((n_) < 631) (nb_) = 19; \
   else if ((n_) < 852) (nb_) = 20; \
   else if ((n_) < 1239) (nb_) = 23; \
   else if ((n_) < 1681) (nb_) = 24; \
   else if ((n_) < 2455) (nb_) = 27; \
   else if ((n_) < 3339) (nb_) = 28; \
   else (nb_) = 30; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
