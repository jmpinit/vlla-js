#ifndef H_SERIAL
#define H_SERIAL

int serial_set_interface_attribs(int fd, int speed, int parity);
void serial_set_blocking(int fd, int should_block);

#endif
