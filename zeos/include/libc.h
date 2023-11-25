/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

int pollKey(char* b);

char* memoryInc(int size);

typedef struct {
int x;
//number of rows
int y;
//number of columns
char* content; //pointer to sprite content matrix(X,Y)
} Sprite;

int spritePut(int posX, int posY, Sprite* sp);

int gotoXY(int posX, int posY);

int SetColor(int color, int background);

int threadCreate(void(*function)(void* arg), void* parameter);


#endif  /* __LIBC_H__ */
