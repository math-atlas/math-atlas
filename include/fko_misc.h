#ifndef FKO_MISC_H
#define FKO_MISC_H

void fko_error(int errno, ...);
void fko_warn(int errno, ...);
int const2shift(int c);
struct loopq *NewLoop(int flag);

#endif
