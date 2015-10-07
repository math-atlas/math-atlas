#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,70,116,207,298,344,390,756,939,1030,1122,1305,1488,2952,5880
 * N : 25,70,116,207,298,344,390,756,939,1030,1122,1305,1488,2952,5880
 * NB : 12,24,32,36,48,60,60,60,60,60,64,68,68,120,140
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 47) (nb_) = 12; \
   else if ((n_) < 93) (nb_) = 24; \
   else if ((n_) < 161) (nb_) = 32; \
   else if ((n_) < 252) (nb_) = 36; \
   else if ((n_) < 321) (nb_) = 48; \
   else if ((n_) < 1076) (nb_) = 60; \
   else if ((n_) < 1213) (nb_) = 64; \
   else if ((n_) < 2220) (nb_) = 68; \
   else if ((n_) < 4416) (nb_) = 120; \
   else (nb_) = 140; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
