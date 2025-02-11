/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <cir_buff.h>

#include <list.h>

#define LECTURA 0
#define ESCRIPTURA 1


void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int global_TID=1000;
extern struct list_head free_thread_lists;
int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /*Save the value of the semaphore*/
  unsigned long sem_addr[NR_SEM];
  for (int i = 0; i < NR_SEM; ++i) sem_addr[i] = (unsigned long)uchild->task.sem[i];
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  page_table_entry *parent_PT = get_PT(current());
  
  int rest = (DWord)(current()->p_heap)%PAGE_SIZE;
  int heap_pag = (((DWord)current()->p_heap - LOG_INIT_HEAP)/PAGE_SIZE);
  if (rest != 0) ++heap_pag;
  
  for (pag=0; pag<NUM_PAG_DATA+heap_pag; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  /* Copy parent's HEAP to child */
  
  new_ph_pag=get_frame(parent_PT, NUM_PAG_KERNEL);
  
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA+heap_pag; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, NUM_PAG_KERNEL, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((NUM_PAG_KERNEL)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, NUM_PAG_KERNEL);
    set_cr3(get_DIR(current()));
  }
  
  set_ss_pag(parent_PT, NUM_PAG_KERNEL, new_ph_pag);
  set_cr3(get_DIR(current()));
  
  /* Deny access to the child's memory space */

  uchild->task.PID=++global_PID;
  uchild->task.TID=++global_TID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /*Restore the value of the semaphore*/
  for (int i = 0; i < NR_SEM; ++i) uchild->task.sem[i] = (struct semaforo*)sem_addr[i];

  /*Assign a thread list*/

  struct list_head* threadL = list_first(&free_thread_lists);
  list_del(threadL);
  INIT_LIST_HEAD(threadL);
  uchild->task.thread_list_ptr = threadL;
  /*Add thread to thread list*/
  list_add_tail(&(uchild->task.thread_head), threadL);
  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}
int threadStacksPage = TOTAL_PAGES;

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());
  int heap_pag = (((DWord)current()->p_heap - LOG_INIT_HEAP)/PAGE_SIZE);
  if ((DWord)current()->p_heap % PAGE_SIZE != 0) ++heap_pag;
  
  //Reiniciar los semáforos
  struct task_struct* act = current();
  
  for (int i = 0; i < NR_SEM; ++i) {
      act->sem[i]->free = 1;
      act->sem[i]->count = -1;
      act->sem[i]->tid_owner = -1;
    }

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA+heap_pag; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  // liberar todos los recursos de todos los threads del proceso
  struct list_head* current_thread_list = current()->thread_list_ptr;
  while (!list_empty(current_thread_list)){
    struct list_head* l = list_first(current_thread_list);
    struct task_struct* thread = list_head_to_task_struct(l);
    list_del(&(thread->list));
    list_add_tail(&(thread->list), &freequeue);
    list_del(&(thread->thread_head));
  }
  list_add_tail(current_thread_list, &free_thread_lists);
  for (i=threadStacksPage; i<TOTAL_PAGES; i++)
  {
    int frame = get_frame(process_PT, i);
    if (frame != 0){
      free_frame(frame);
      del_ss_pag(process_PT, i);
    }
  }
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

extern struct buffer_cir keyBuff;

int sys_pollKey(char* b){
	if (!access_ok(VERIFY_WRITE, b, 1))
		return -EFAULT;
  char c;
  int ret = CircularBufferRead(&keyBuff, &c);
  copy_to_user(&c, b, sizeof(char));
  return ret;
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}



int sys_threadCreate(void(*function)(void* arg), void* parameter){
  struct list_head *lhcurrent = NULL;
  union task_union *newThread;
  //comprovar que funcion este en la zona de codigo
  if ((unsigned long) function <= ((USER_FIRST_PAGE) << 12) || (unsigned long) function >= ((USER_FIRST_PAGE + NUM_PAG_CODE) << 12)) return -EFAULT;
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  /*Copy t_u current*/
  newThread=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  copy_data(current(), newThread, sizeof(union task_union));

  page_table_entry *newThread_PT = get_PT(&newThread->task);

  /*Busqueda de la primera pagina libre desde el final de memoria*/
  int i = 0;
  while (i < TOTAL_PAGES && get_frame(newThread_PT, ((TOTAL_PAGES - 1) - i)) != 0){
    i++;
  }
  if (i == TOTAL_PAGES){
    /* Free task_struct */
    list_add_tail(&(newThread->task.list), &freequeue);
    return -EAGAIN;
  }

  /*Reserva y mapeo de pila de usuario*/
  int frame = alloc_frame();
  if (frame == -1){
    /* Free task_struct */
    list_add_tail(&(newThread->task.list), &freequeue);
    return -EAGAIN;
  }
  unsigned long last_page = ((TOTAL_PAGES - 1) - i);
  set_ss_pag(newThread_PT, last_page, frame);
  DWord user_stackAddr = (last_page <<12);
  if (last_page < threadStacksPage) threadStacksPage = last_page;
  user_stackAddr += 0xfff;
  /*Init ctx ejecucion*/
  *(DWord*)(user_stackAddr - 4) = (DWord) parameter;
  *(DWord*)(user_stackAddr - 8) = (DWord) 0;
  newThread->stack[KERNEL_STACK_SIZE - 2]/*esp*/ = user_stackAddr - 8;
  newThread->stack[KERNEL_STACK_SIZE - 5]/*eip*/ = (unsigned long)function;
  /* Set stats to 0 */
  init_stats(&(newThread->task.p_stats));

  newThread->task.TID=++global_TID;
  /*Queue Thread*/
  list_add_tail(&(newThread->task.thread_head), newThread->task.thread_list_ptr);
  /*Queue into readyqueue*/
  newThread->task.state=ST_READY;
  list_add_tail(&(newThread->task.list), &readyqueue);
  /*update kernel esp*/
	newThread->task.register_esp = (unsigned long) &(newThread->stack[KERNEL_STACK_SIZE-18]); 

  return newThread->task.TID;
}

void sys_threadExit(void){
  // Deallocate stack physical page
  struct task_struct* act = current();
  page_table_entry *process_PT = get_PT(act);
  //dealloc user stack
  DWord stack_page = ((((union task_union*)act)->stack[KERNEL_STACK_SIZE - 2]/*esp*/)>>12);

  free_frame(get_frame(process_PT, stack_page));
  del_ss_pag(process_PT, stack_page);

  int heap_pag = (((DWord)current()->p_heap - LOG_INIT_HEAP)/PAGE_SIZE);
  if ((DWord)current()->p_heap % PAGE_SIZE != 0) ++heap_pag;

  //si es el ultimo thread de la lista deallocata mem
  list_del(&(act->thread_head));
  if (list_empty(act->thread_list_ptr)){
    list_add_tail(act->thread_list_ptr, &free_thread_lists);
    // Deallocate all the propietary physical pages
    for (int i=0; i<NUM_PAG_DATA+heap_pag; i++)
    {
      free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
      del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
    }
  }
  /* Free task_struct */
  list_del(&act->list);
  list_add_tail(&(act->list), &freequeue);

  /* Restarts execution of the next process */
  sched_next_rr();
}

char* sys_memoryInc(int size) {	
  if (size <= 0) return (char*)NULL;
  page_table_entry *process_PT = get_PT(current());
  struct task_struct* act = current();
  int new_ph_pag, pag_act, rest, num_pag = 0;
  
  rest = (DWord)(act->p_heap)%PAGE_SIZE;
  num_pag += (rest + size) >> 12;
  if (rest == 0 && (size%PAGE_SIZE) != 0) ++num_pag;
  else if (rest != 0 && (size%PAGE_SIZE) != 0 && (rest+size)%PAGE_SIZE == 0) --num_pag;
  
  pag_act = (DWord)act->p_heap / PAGE_SIZE;
  if (rest != 0) ++pag_act;
  if (num_pag+pag_act >= TOTAL_PAGES) return (char*)NULL;
  
  for (int pag = 0; pag < num_pag; pag++) {
    new_ph_pag = alloc_frame();

    if (new_ph_pag != -1 && get_frame(process_PT, pag_act+pag) == 0) set_ss_pag(process_PT, pag_act+pag, new_ph_pag);
    else {
    
      for (int i = 0; i < pag; i++) {
        free_frame(get_frame(process_PT, pag_act+i));
        del_ss_pag(process_PT, pag_act+i);
      }
      return (char*)NULL; 
    }
  }
  char* in = act->p_heap;
  act->p_heap += size;
  return in;
}
#define NUM_COLUMNS 80
#define NUM_ROWS    25

typedef struct {
  int x;
//number of rows
  int y;
//number of columns
  char* content; //pointer to sprite content matrix(X,Y)
} Sprite;

int sys_spritePut(int posX, int posY, Sprite* sp) {
  if (!access_ok(VERIFY_READ, sp, sizeof(Sprite))) return -EAGAIN;
  if (!access_ok(VERIFY_READ, sp->content, sp->x*sp->y)) return -EAGAIN;
  if (posX < 0 || posX+sp->x >= NUM_COLUMNS || posY < 0 || posY+sp->y >= NUM_ROWS) return -EAGAIN;
  if (sp->x == 0 || sp->y == 0) return -EAGAIN;
  if (sp->content == (char*)NULL) return -EAGAIN;
  
  int cont = 0;
  for (int i = 0; i < sp->y; ++i) {
    for (int j = 0; j < sp->x; ++j) {
      printc_xy(posX+i, posY+j, (char)sp->content[cont]);
      ++cont;
    }
  }
  return 1;
}

extern Byte x, y;

int sys_gotoXY(int posX, int posY) {
  if (posX < 0 || posX >= NUM_COLUMNS || posY < 0 || posY >= NUM_ROWS) return -EAGAIN;
  x = posX;
  y = posY;
  return 1;
}

Word col = 0x0200;

int sys_SetColor(int color, int background) {
  if (color < 0 || color > 15 || background < 0 || background > 15) return -EAGAIN;
  col = (background << 12) + (color << 8);
  return 1;
}

//SEMAFOROS

int test(int semid) {
  struct task_struct* act = current();
  
  if (semid < 0 || semid >= NR_SEM) return -1;
  if (act->sem[semid]->free == 1) return -1;
  return 1;
}

int sys_semCreate(int initial_value) {
  struct task_struct* act = current();
  
  if (initial_value < 0) return -1;
  int find = -1;
  for (int i = 0; i < NR_SEM && find == -1; ++i) {
    if (act->sem[i]->count == -1) {
      act->sem[i]->free = 0;
      act->sem[i]->count = initial_value;
      act->sem[i]->tid_owner = act->TID;
      find = i;
      INIT_LIST_HEAD(&(act->sem[i]->sem_list));
    }
  }
  if (find == -1) return -1;
  return find;
}

int sys_semWait(int semid) {
  struct task_struct* act = current();
  
  if (test(semid) == -1) return -1;
  
  act->sem[semid]->count -= 1;
  
  if (act->sem[semid]->count < 0) {
    list_add_tail(&(act->list), &(act->sem[semid]->sem_list));
    sched_next_rr();
  }
  
  return 1;
}

int sys_semSignal(int semid) {
  struct task_struct* act = current();
  
  if (test(semid) == -1) return -1;
  
  act->sem[semid]->count += 1;
  if (act->sem[semid]->count <= 0) {
    struct list_head *new = list_first(&(act->sem[semid]->sem_list));
    list_del(new);
    list_add_tail(new, &readyqueue);
  }
  return 1;
}

int sys_semDestroy(int semid) {
  struct task_struct* act = current();
  
  if (test(semid) == -1) return -1;
	
  if (act->TID == act->sem[semid]->tid_owner) {
    act->sem[semid]->free = 1;
    
    while(!list_empty(&(act->sem[semid]->sem_list))) {
      struct list_head * new = list_first(&(act->sem[semid]->sem_list));
      list_del(new);
      list_add_tail(new, &readyqueue);
    }
  }
  else return -1;
  
  return 1;
}
