#include <libc.h>

char buff[24];

int pid;

int semid;

void readKeyBoard(){
  semSignal(semid);
  char b;
  while (1){
    if (pollKey(&b) > -1){
      write(1, &b, 1);
    }
  }
}

void exitThread(){
  threadExit();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  Sprite sp;
  sp.x = 3;
  sp.y = 1;
  sp.content = "---";
  spritePut(20, 20, &sp);
  itoa(0,buff);
  char* buff1 = memoryInc(96);
  memoryInc(4000);
  semid = semCreate(2);
  int pid = fork();
  if (pid == 0){
    char buff2[32];
    itoa((int)buff1, buff2);
    gotoXY(30, 20);
    SetColor(5, 7);
    write(1,buff2, strlen(buff2));
    threadCreate(&exitThread, (void*)0);
    semDestroy(semid);
    semWait(semid);
    semWait(semid);
    semWait(semid);
    char suu[20] = "No me he bloqueado";
    write(1,suu, strlen(suu));
    threadExit();
  }
  else {
    char* buff3 = memoryInc(96);
    char buff2[32];
    itoa((int)buff3, buff2);
    write(1,buff2, strlen(buff2));
    threadCreate(&readKeyBoard, (void*)0);
    semWait(semid);
    semWait(semid);
    semWait(semid);
    char klk[10] = "yayyy";
    write(1, klk, strlen(klk));
    pid = fork();
    if (pid == 0) exit();
  }
  while(1) { 

  }
}
