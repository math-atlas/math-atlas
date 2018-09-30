#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,150,212,275,400,526,651,777,902,1028,1530,1781,2032,3036,4040
 * N : 25,87,150,212,275,400,526,651,777,902,1028,1530,1781,2032,3036,4040
 * NB : 1,1,23,20,35,39,83,82,95,95,99,95,95,186,372,372
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 1; \
   else if ((n_) < 181) (nb_) = 23; \
   else if ((n_) < 243) (nb_) = 20; \
   else if ((n_) < 337) (nb_) = 35; \
   else if ((n_) < 463) (nb_) = 39; \
   else if ((n_) < 588) (nb_) = 83; \
   else if ((n_) < 714) (nb_) = 82; \
   else if ((n_) < 965) (nb_) = 95; \
   else if ((n_) < 1279) (nb_) = 99; \
   else if ((n_) < 1906) (nb_) = 95; \
   else if ((n_) < 2534) (nb_) = 186; \
   else (nb_) = 372; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
