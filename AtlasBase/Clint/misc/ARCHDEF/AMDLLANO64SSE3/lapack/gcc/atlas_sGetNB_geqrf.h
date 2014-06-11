#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,135,190,245,355,466,687,908,1792,3560
 * N : 25,80,135,190,245,355,466,687,908,1792,3560
 * NB : 8,8,12,12,24,24,36,44,84,112,168
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 107) (nb_) = 8; \
   else if ((n_) < 217) (nb_) = 12; \
   else if ((n_) < 410) (nb_) = 24; \
   else if ((n_) < 576) (nb_) = 36; \
   else if ((n_) < 797) (nb_) = 44; \
   else if ((n_) < 1350) (nb_) = 84; \
   else if ((n_) < 2676) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
