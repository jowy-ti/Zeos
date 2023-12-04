#include <libc.h>

char buff[24];

int pid;

void readKeyBoard(){
  char b;
  while (1){
    if (pollKey(&b) > -1){
      write(1, &b, 1);
    }
  }
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
  spritePut(50, 20, &sp);
  char* buff1 = memoryInc(96);
  memoryInc(4000);
  int pid = fork();
  if (pid == 0){
    char buff2[32];
    itoa((int)buff1, buff2);
    gotoXY(30, 20);
    SetColor(5, 7);
    write(1,buff2, strlen(buff2));
    threadCreate(&readKeyBoard, (void*)0);
  }
  else {
    char* buff3 = memoryInc(96);
    char buff2[32];
    itoa((int)buff3, buff2);
    write(1,buff2, strlen(buff2));
    threadCreate(&readKeyBoard, (void*)0);
  }
  while(1) { 

  }
}
