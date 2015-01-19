#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,81,137,250,306,362,418,475,531,587,643,700,813,869,926,1828,1940,2053,2279,2730,3632,7240
 * N : 25,81,137,250,306,362,418,475,531,587,643,700,813,869,926,1828,1940,2053,2279,2730,3632,7240
 * NB : 8,24,24,24,32,36,44,48,48,72,72,76,76,80,80,80,96,96,96,96,164,168
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 53) (nb_) = 8; \
   else if ((n_) < 278) (nb_) = 24; \
   else if ((n_) < 334) (nb_) = 32; \
   else if ((n_) < 390) (nb_) = 36; \
   else if ((n_) < 446) (nb_) = 44; \
   else if ((n_) < 559) (nb_) = 48; \
   else if ((n_) < 671) (nb_) = 72; \
   else if ((n_) < 841) (nb_) = 76; \
   else if ((n_) < 1884) (nb_) = 80; \
   else if ((n_) < 3181) (nb_) = 96; \
   else if ((n_) < 5436) (nb_) = 164; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
