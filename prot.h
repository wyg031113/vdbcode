#ifndef __PROT_H__
#define __PROT_H__

#define T_FILE_Q 1
#define T_FILE_HIJ 2
#define T_QUERY 3
#define T_FILE_RECVD 4
struct packet
{
    int type;
    unsigned int len;
    char value[0];
}/*__attribute_packed__(1)*/;
#endif /*__PROT_H__*/
