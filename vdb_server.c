#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#include "io.h"
#include "prot.h"
#define FILE_NAME_LEN 128
unsigned short port = 7788;
char q_file[FILE_NAME_LEN]="./sparam/q";
char hij_file[FILE_NAME_LEN]="./sparam/hij";
int start_listen(void)
{
    int listen_fd = -1;
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    if((listen_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket failed!\n");
        exit(-1);
    }

    if(bind(listen_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr))!=0)
    {
        printf("Bind failed!\n");
        exit(-1);
    }
    if(listen(listen_fd, 5) != 0)
    {
        printf("Listen failed!\n");
        exit(-1);
    }
    return listen_fd;

}
#define BUF_SIZE 2048
static char rcv_buf[BUF_SIZE];


/**
 * pkt len is BUF_SIZE
 */
int handle_pkt(int fd, struct packet *pkt)
{
    int ret = 0;
    switch(pkt->type)
    {
        case T_FILE_Q:
            printf("Receive file q.\n");
            return recv_file(fd, q_file, pkt->len) == pkt->len;
            break;
        case T_FILE_HIJ:
            printf("Receive file hij.\n");
            return recv_file(fd, hij_file, pkt->len) == pkt->len;
            break;
        case T_QUERY:
            printf("query.\n");
            break;
        default:
            return 0;
    }
    return 1;
}
void handle_client(int fd)
{
    char wel[] = "Welcome to use vdb server.\n";
    const int len = sizeof(struct packet);
    struct packet *pkt = (struct packet *)rcv_buf;
    write(fd, wel, sizeof(wel));
    while(1)
    {
        if(read_all(fd, (char*)pkt, len) != len)
        {
            printf("read pkt failed!\n");
            break;
        }
        if(!handle_pkt(fd, pkt))
        {
            printf("Handle pkt failed!\n");
            break;
        }
    }
    close(fd);
}
void run_server(int fd)
{
    int client_fd = -1;
    printf("Server is running.\n");
    while(1)
    {
        client_fd = accept(fd, NULL, 0);
        if(fd<0)
        {
            printf("accpet failed!\n");
            exit(-1);
        }
        printf("accept a client.fd=%d\n", client_fd);
        handle_client(client_fd);
    }
}
int main()
{

    run_server(start_listen());
    return 0;
}
