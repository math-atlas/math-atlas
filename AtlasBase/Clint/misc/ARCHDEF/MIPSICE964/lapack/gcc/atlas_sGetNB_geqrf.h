#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,50,75,100,125,150,175,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1600,1800,2000
 * N : 25,50,75,100,125,150,175,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1600,1800,2000
 * NB : 1,8,8,8,12,12,12,16,16,24,24,28,24,28,24,28,36,36,32,40,40,40,48,48
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 37) (nb_) = 1; \
   else if ((n_) < 112) (nb_) = 8; \
   else if ((n_) < 187) (nb_) = 12; \
   else if ((n_) < 275) (nb_) = 16; \
   else if ((n_) < 375) (nb_) = 24; \
   else if ((n_) < 425) (nb_) = 28; \
   else if ((n_) < 475) (nb_) = 24; \
   else if ((n_) < 550) (nb_) = 28; \
   else if ((n_) < 650) (nb_) = 24; \
   else if ((n_) < 750) (nb_) = 28; \
   else if ((n_) < 950) (nb_) = 36; \
   else if ((n_) < 1100) (nb_) = 32; \
   else if ((n_) < 1700) (nb_) = 40; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
