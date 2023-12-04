#ifndef _SEMAFORO
#define _SEMAFORO

#include <list.h>

struct semaforo {
  int count;
  struct list_head block_list;
};

void block(struct list_head* list, struct semaforo *s);


#endif 
