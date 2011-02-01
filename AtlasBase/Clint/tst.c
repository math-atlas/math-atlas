#include <pthread.h>

void bob()
{
   pthread_cond_t condvar;
   pthread_cond_t *cond;
   cond = &condvar;
}
