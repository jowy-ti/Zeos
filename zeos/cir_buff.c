#include <cir_buff.h>



void CircularBufferInit(struct buffer_cir* buf){
	buf->write_ptr = 0;
	buf->read_ptr = 0;
	buf->num_elem = 0;
}

int CircularBufferIsFull(struct buffer_cir* buf){
	return buf->num_elem == BUF_SIZE;
}

int CircularBufferIsEmpty(struct buffer_cir* buf){
	return buf->num_elem == 0;
}

void CircularBufferWrite (struct buffer_cir* buf, char c){
	if (!CircularBufferIsFull(buf)){
		buf->buffer[buf->write_ptr] = c;
		buf->write_ptr = (buf->write_ptr + 1) % BUF_SIZE;
		++buf->num_elem;
	}
}

int CircularBufferRead (struct buffer_cir* buf, char* c){
	if (!CircularBufferIsEmpty(buf)){
		*c = buf->buffer[buf->read_ptr];
		buf->read_ptr = (buf->read_ptr + 1) % BUF_SIZE;
		--buf->num_elem;
		return 0;
	}
	return -1;
}
