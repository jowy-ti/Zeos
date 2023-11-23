#ifndef __CIR_BUFF_H__
#define __CIR_BUFF_H__

#define BUF_SIZE 256

struct buffer_cir
{
	int write_ptr;
	int read_ptr;
	int num_elem;
	char buffer[BUF_SIZE];
};

void CircularBufferInit(struct buffer_cir* buf);
void CircularBufferWrite (struct buffer_cir* buf, char c);
int CircularBufferIsFull(struct buffer_cir* buf);
int CircularBufferRead (struct buffer_cir* buf, char* c);
int CircularBufferIsEmpty(struct buffer_cir* buf);

#endif