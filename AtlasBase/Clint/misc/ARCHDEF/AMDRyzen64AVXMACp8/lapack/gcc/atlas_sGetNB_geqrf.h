#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,66,108,150,192,276,360,528,570,612,654,696,1032,1368,2040,2712,4056,4728,5400
 * N : 25,66,108,150,192,276,360,528,570,612,654,696,1032,1368,2040,2712,4056,4728,5400
 * NB : 1,4,11,15,23,18,35,32,35,36,36,51,59,99,131,195,192,216,227
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 45) (nb_) = 1; \
   else if ((n_) < 87) (nb_) = 4; \
   else if ((n_) < 129) (nb_) = 11; \
   else if ((n_) < 171) (nb_) = 15; \
   else if ((n_) < 234) (nb_) = 23; \
   else if ((n_) < 318) (nb_) = 18; \
   else if ((n_) < 444) (nb_) = 35; \
   else if ((n_) < 549) (nb_) = 32; \
   else if ((n_) < 591) (nb_) = 35; \
   else if ((n_) < 675) (nb_) = 36; \
   else if ((n_) < 864) (nb_) = 51; \
   else if ((n_) < 1200) (nb_) = 59; \
   else if ((n_) < 1704) (nb_) = 99; \
   else if ((n_) < 2376) (nb_) = 131; \
   else if ((n_) < 3384) (nb_) = 195; \
   else if ((n_) < 4392) (nb_) = 192; \
   else if ((n_) < 5064) (nb_) = 216; \
   else (nb_) = 227; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
