//#include <utils.h>

//#include <io.h>

//#include <mm.h>

//#include <mm_address.h>

//#include <errno.h>

#include <semaforo.h>
#include <sched.h>

void block(struct list_head* list, struct semaforo *s) {

  list_add_tail(list, &(s->block_list));
  sched_next_rr();

}
