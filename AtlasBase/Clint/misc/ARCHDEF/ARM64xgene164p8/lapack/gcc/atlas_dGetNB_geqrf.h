#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,97,170,206,243,279,316,389,462,535,608,754,900,1046,1192,1338,1484,1630,1776,2068,2360
 * N : 25,97,170,206,243,279,316,389,462,535,608,754,900,1046,1192,1338,1484,1630,1776,2068,2360
 * NB : 1,4,15,15,16,20,23,23,24,24,27,27,28,28,35,35,36,48,59,75,99
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 61) (nb_) = 1; \
   else if ((n_) < 133) (nb_) = 4; \
   else if ((n_) < 224) (nb_) = 15; \
   else if ((n_) < 261) (nb_) = 16; \
   else if ((n_) < 297) (nb_) = 20; \
   else if ((n_) < 425) (nb_) = 23; \
   else if ((n_) < 571) (nb_) = 24; \
   else if ((n_) < 827) (nb_) = 27; \
   else if ((n_) < 1119) (nb_) = 28; \
   else if ((n_) < 1411) (nb_) = 35; \
   else if ((n_) < 1557) (nb_) = 36; \
   else if ((n_) < 1703) (nb_) = 48; \
   else if ((n_) < 1922) (nb_) = 59; \
   else if ((n_) < 2214) (nb_) = 75; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
