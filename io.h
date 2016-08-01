#ifndef __SOCKET_IO_H__
#define __SOCKET_IO_H__
int write_all(int fd, const char *buf, int len);
int read_all(int fd, char *buf, int len);
int read_all_s(int fd, char *buf, int len);
int get_file_len(const char *file);
#endif /*SOCKET_IO_H__*/
