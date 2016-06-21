#ifndef ATLAS_CACHEEDGE_H
   #define ATLAS_CACHEEDGE_H
   #define CacheEdge 262144
   #ifdef DCPLX
      #undef CacheEdge
      #define CacheEdge 1048576
   #endif
#endif
