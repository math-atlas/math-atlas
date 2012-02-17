#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,504,576,648,720,864,936,1080,1296,1728
 * N : 25,216,432,504,576,648,720,864,936,1080,1296,1728
 * NB : 1,16,16,16,16,36,72,40,136,72,56,72
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 1; \
   else if ((n_) < 612) (nb_) = 16; \
   else if ((n_) < 684) (nb_) = 36; \
   else if ((n_) < 792) (nb_) = 72; \
   else if ((n_) < 900) (nb_) = 40; \
   else if ((n_) < 1008) (nb_) = 136; \
   else if ((n_) < 1188) (nb_) = 72; \
   else if ((n_) < 1512) (nb_) = 56; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
