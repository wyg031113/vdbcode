#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "io.h"
#include "prot.h"

#define FILE_NAME_LEN 128
static char serip[17]="127.0.0.1";
static unsigned short port = 7788;
char hij_file[FILE_NAME_LEN] = "./param/hij";
int connect_server()
{
    int ser = -1;
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = inet_addr(serip);
    if((ser=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket failed in connect server!\n");
        exit(-1);
    }
    if((ser==connect(ser, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr))) < 0)
    {
        printf("Connect server failed!\n");
        exit(-1);
    }

    return ser;
}

int send_hij(int fd, int type, const char *file_name)
{
    struct packet pkt;
    pkt.type = type;
    pkt.len = get_file_len(file_name);
    if(pkt.len == -1)
    {
        printf("Bad file len.\n");
        return -1;
    }
    if(write_all(fd, (char *)&pkt, sizeof(pkt)) != sizeof(pkt))
    {
        printf("write header hij file failed!\n");
        return -1;
    }
    if(send_file(fd, file_name, pkt.len) != pkt.len)
    {
        printf("write hij file content failed!\n");
        return -1;
    }
    return 0;

}
int send_param_file(int fd, int type)
{

    switch(type)
    {
        case T_FILE_Q:
            break;
        case T_FILE_HIJ:
            return send_hij(fd, T_FILE_HIJ, hij_file);
            break;
        default:
            return -1;
    }
    return -1;
}
int main()
{
    int sd = connect_server();
    send_param_file(sd, T_FILE_HIJ);
    sleep(2);
    close(sd);
    return 0;
}
