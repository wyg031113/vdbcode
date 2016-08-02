#ifndef __PROT_H__
#define __PROT_H__

#define T_FILE_Q 1
#define T_FILE_HIJ 2
#define T_QUERY 3
#define T_FILE_RECVD 4
#define T_FILE_T 5
#define T_FILE_CU0 6
#define T_FILE_Cf1 7
#define T_FILE_H0 8
#define T_FILE_C0 9
#define T_FINISH 10
#define T_REPLY_QUERY 11
#define T_HIJ 12
#define T_FILE_APARAM 13
#define P_UNINIT 1
#define P_INITING 2
#define P_FINISH 3

#define Q_ING 1
#define Q_FAILED 2
#define Q_SUCCESS 3
struct packet
{
    int type;
    unsigned int len;
    char value[0];
}/*__attribute_packed__(1)*/;
#endif /*__PROT_H__*/
