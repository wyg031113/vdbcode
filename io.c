#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

/*向fd中写入len字节数据，直到完全写入或者出错
 * 才返回。
 * return:实际写入字节数
 */
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


/*向fd中读取len字节数据，直到完全读入或者出错
 * 才返回。
 * return:实际读入字节数
 */
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

/**
 * 用recv实现读入功能
 */
int read_all_s(int fd, char *buf, int len)
{
    int ret = 0;
    int real_read = 0;
    while(len > 0)
    {
        ret = recv(fd, buf+real_read, len);
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

/*从fd接收文件，并保存到fname中，文件长度len
 * return:实际接收文件长度
 * 出错返回-1
 */
#define BUF_SIZE 2048
int recv_file(int fd, const char *fname, int len)
{
    static char data[BUF_SIZE]; //注意，静态成员
    int real_recv = 0;
    printf("rcv file %s len = %d\n", fname, len);
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
   // printf("rcv real file len = %d\n", real_recv);
    close(file_fd);
    return real_recv;
}

/*向fd发送文件fname，文件长度len
 * return:实际发送文件长度
 * 出错返回-1
 */
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
    printf("send file %s len = %d\n", fname, len);
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
    //printf("read send file len = %d\n", real_send);
    return real_send;
}

/**获取文件长度
 */
int get_file_len(const char *file)
{
    struct stat st;
    if(stat(file, &st) != 0)
        return -1;
    return st.st_size;
}
/**切换运行路径到可执行文件所在路径
 * arg0是main函数的argv[0]
 */
void change_dir(char *arg0)
{
    int i = 0;
    char dir_buf[256];
    if(arg0[0] == '/')
    {
        i = strlen(arg0)-1;
        while(i>0 && arg0[i] != '/')
            i--;
        snprintf(dir_buf, i+1,"%s", arg0);
        printf("up change pwd to dir:%s\n", dir_buf);
        chdir(dir_buf);
    }
    else
    {
        if(!(arg0[0] == '.' && arg0[1]=='.'))
        while(arg0[i] == '.' || arg0[i] == '/')
            i++;
        snprintf(dir_buf, 256,"%s/%s",getenv("PWD"), arg0+i);

        i = strlen(dir_buf)-1;
        while(i>0 && dir_buf[i] != '/')
            i--;
        dir_buf[i] = '\0';
        printf("down change pwd to dir:%s arg0=%s\n", dir_buf, arg0);
        chdir(dir_buf);
    }
    setenv("PWD", dir_buf, 1);
}

