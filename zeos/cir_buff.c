#include <cir_buff.h>



void CircularBufferInit(buffer_cir* buf){
	buf->write_ptr = 0;
	buf->read_ptr = 0;
	buf->num_elem = 0;
}

int CircularBufferIsFull(buffer_cir* buf){
	return (((buf->write_ptr + 1) % BUF_SIZE) == buf->read_ptr);
}

int CircularBufferIsEmpty(buffer_cir* buf){
	return (buf->read_ptr == buf->write_ptr);
}

int CircularBufferEnque (buffer_cir* buf, char c){
	int full = CircularBufferIsFull	(buf);
	if (!full){
		buf->buffer[buf->write_ptr] = c;
		buf->write_ptr = (buf->write_ptr + 1) % BUF_SIZE;
		++buf->num_elem;
	}
}
