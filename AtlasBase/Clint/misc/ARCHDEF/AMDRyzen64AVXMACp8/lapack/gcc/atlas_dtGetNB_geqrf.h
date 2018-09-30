#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,210,272,334,396,458,520,582,644,706,768,892,1016,1512,2008,3000,3992,7960
 * N : 25,86,148,210,272,334,396,458,520,582,644,706,768,892,1016,1512,2008,3000,3992,7960
 * NB : 1,1,23,19,35,35,36,48,51,55,59,59,115,110,131,130,163,183,576,576
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 1; \
   else if ((n_) < 179) (nb_) = 23; \
   else if ((n_) < 241) (nb_) = 19; \
   else if ((n_) < 365) (nb_) = 35; \
   else if ((n_) < 427) (nb_) = 36; \
   else if ((n_) < 489) (nb_) = 48; \
   else if ((n_) < 551) (nb_) = 51; \
   else if ((n_) < 613) (nb_) = 55; \
   else if ((n_) < 737) (nb_) = 59; \
   else if ((n_) < 830) (nb_) = 115; \
   else if ((n_) < 954) (nb_) = 110; \
   else if ((n_) < 1264) (nb_) = 131; \
   else if ((n_) < 1760) (nb_) = 130; \
   else if ((n_) < 2504) (nb_) = 163; \
   else if ((n_) < 3496) (nb_) = 183; \
   else (nb_) = 576; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
