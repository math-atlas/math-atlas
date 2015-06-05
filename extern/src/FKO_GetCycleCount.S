#include "fko_settings"
#if X86
   .text
.global GetCycleCount
GetCycleCount:
   #ifdef __LP64__
      xorq %rax, %rax
      .byte 0x0f; .byte 0x31
      shlq  $32, %rdx
      orq   %rdx, %rax
   #else
      .byte 0x0f; .byte 0x31
   #endif
    ret
#elif defined(SPARC)
   .section        ".text"
   .global GetCycleCount
GetCycleCount:
   rd      %tick, %o0
   clruw   %o0, %o1 
   retl
   srlx    %o0, 32, %o0
#endif
