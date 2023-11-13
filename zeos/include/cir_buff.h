#define BUF_SIZE 256

struct buffer_cir
{
	int write_ptr;
	int read_ptr;
	int num_elem;
	int buffer[BUF_SIZE];
};

void CircularBufferInit(buffer_cir* buf);
void CircularBufferEnque (buffer_cir* buf);
void CircularBufferIsFull(buffer_cir* buf)
