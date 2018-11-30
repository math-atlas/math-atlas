#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,57,90,122,155,220,285,317,350,382,415,447,480,512,545,805,1066,1587,2108,4192,6276,8360
 * N : 25,57,90,122,155,220,285,317,350,382,415,447,480,512,545,805,1066,1587,2108,4192,6276,8360
 * NB : 1,3,4,8,9,9,10,12,14,18,27,28,59,48,99,107,132,104,384,384,480,768
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 41) (nb_) = 1; \
   else if ((n_) < 73) (nb_) = 3; \
   else if ((n_) < 106) (nb_) = 4; \
   else if ((n_) < 138) (nb_) = 8; \
   else if ((n_) < 252) (nb_) = 9; \
   else if ((n_) < 301) (nb_) = 10; \
   else if ((n_) < 333) (nb_) = 12; \
   else if ((n_) < 366) (nb_) = 14; \
   else if ((n_) < 398) (nb_) = 18; \
   else if ((n_) < 431) (nb_) = 27; \
   else if ((n_) < 463) (nb_) = 28; \
   else if ((n_) < 496) (nb_) = 59; \
   else if ((n_) < 528) (nb_) = 48; \
   else if ((n_) < 675) (nb_) = 99; \
   else if ((n_) < 935) (nb_) = 107; \
   else if ((n_) < 1326) (nb_) = 132; \
   else if ((n_) < 1847) (nb_) = 104; \
   else if ((n_) < 5234) (nb_) = 384; \
   else if ((n_) < 7318) (nb_) = 480; \
   else (nb_) = 768; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
