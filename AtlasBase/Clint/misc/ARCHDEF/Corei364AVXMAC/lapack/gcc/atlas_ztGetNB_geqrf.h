#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,82,140,197,255,312,370,486,601,717,948,1872,2103,2334,2796,3027,3258,3489,3720
 * N : 25,82,140,197,255,312,370,486,601,717,948,1872,2103,2334,2796,3027,3258,3489,3720
 * NB : 8,12,12,32,32,32,40,40,44,48,48,60,60,64,64,68,96,104,104
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 53) (nb_) = 8; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 341) (nb_) = 32; \
   else if ((n_) < 543) (nb_) = 40; \
   else if ((n_) < 659) (nb_) = 44; \
   else if ((n_) < 1410) (nb_) = 48; \
   else if ((n_) < 2218) (nb_) = 60; \
   else if ((n_) < 2911) (nb_) = 64; \
   else if ((n_) < 3142) (nb_) = 68; \
   else if ((n_) < 3373) (nb_) = 96; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
