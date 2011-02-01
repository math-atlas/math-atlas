#ifndef ATLAS_CACHEEDGE_H
   #define ATLAS_CACHEEDGE_H
   #define CacheEdge 131072
   #ifdef DCPLX
      #undef CacheEdge
      #define CacheEdge 262144
   #elif defined(SREAL)
      #undef CacheEdge
      #define CacheEdge 196608
/*      #define CacheEdge 65536 */
   #endif
#endif
