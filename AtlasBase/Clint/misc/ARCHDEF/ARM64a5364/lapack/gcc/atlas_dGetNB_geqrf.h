#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,432,864,1296,1536,1632,1680,1728,1776
 * N : 25,432,864,1296,1536,1632,1680,1728,1776
 * NB : 12,12,28,28,28,28,28,28,96
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 648) (nb_) = 12; \
   else if ((n_) < 1752) (nb_) = 28; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
