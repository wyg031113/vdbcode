#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int write_all(int fd, const char *buf, int len)
{
    int ret = 0;
    int real_write = 0;
    while(len > 0)
    {
        ret = write(fd, buf + real_write, len);
        if(ret <= 0)
        {
            printf("write error:%s\n", strerror(errno));
            break;
        }
        real_write += ret;
        len -= ret;

    }
    return real_write;
}

int read_all(int fd, char *buf, int len)
{
    int ret = 0;
    int real_read = 0;
    while(len > 0)
    {
        ret = read(fd, buf+real_read, len);
        if(ret <= 0)
        {
            printf("read error:%s\n", strerror(errno));
            break;
        }
        real_read += ret;
        len -= ret;

    }
    return real_read;
}
#define BUF_SIZE 2048
int recv_file(int fd, const char *fname, int len)
{
    char data[BUF_SIZE];
    int real_recv = 0;
    printf("rcv file len = %d\n", len);
    int file_fd = open(fname, O_WRONLY|O_CREAT, 0666);
    if(file_fd < 0)
    {
        printf("open file %s failed!\n", fname);
        return -1;
    }
    while(len > 0)
    {
        int rbytes = len < BUF_SIZE ? len : BUF_SIZE;
        if(read_all(fd, data, rbytes) != rbytes)
            break;
        if(write_all(file_fd, data, rbytes) != rbytes)
            break;
        len -= rbytes;
        real_recv += rbytes;
    }
    printf("rcv real file len = %d\n", real_recv);
    close(file_fd);
    return real_recv;
}

int send_file(int fd, const char *fname, int len)
{
    char data[BUF_SIZE];
    int real_send = 0;
    int file_fd = open(fname, O_RDONLY);

    if(file_fd < 0 )
    {
        printf("open file %s failed!\n", fname);
        return -1;
    }
    printf("file len = %d\n", len);
    while(len > 0)
    {
        int rbytes = len < BUF_SIZE ? len : BUF_SIZE;
        if(read_all(file_fd, data, rbytes) != rbytes)
            break;
        if(write_all(fd, data, rbytes) != rbytes)
            break;
        len -= rbytes;
        real_send += rbytes;
    }
    close(file_fd);
    printf("read send file len = %d\n", real_send);
    return real_send;
}

int get_file_len(const char *file)
{
    struct stat st;
    if(stat(file, &st) != 0)
        return -1;
    return st.st_size;
}
