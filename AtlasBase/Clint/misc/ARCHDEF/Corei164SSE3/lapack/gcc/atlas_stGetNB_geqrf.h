#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,360,504,576,720,1080,1152,1224,1296,1512,3096,6192
 * N : 25,360,504,576,720,1080,1152,1224,1296,1512,3096,6192
 * NB : 2,2,2,168,144,144,168,144,192,204,192,144
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 540) (nb_) = 2; \
   else if ((n_) < 648) (nb_) = 168; \
   else if ((n_) < 1116) (nb_) = 144; \
   else if ((n_) < 1188) (nb_) = 168; \
   else if ((n_) < 1260) (nb_) = 144; \
   else if ((n_) < 1404) (nb_) = 192; \
   else if ((n_) < 2304) (nb_) = 204; \
   else if ((n_) < 4644) (nb_) = 192; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
