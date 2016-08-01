#include <pbc/pbc.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include "simple_db.h"

#define MAX_SQL_LEN 1024
static mpz_t *sdb;
static int  db_size;
char user[128] = "root";
char passwd[128] = "letmein";//"admin";
char db[128] = "dbtest";
char host[128]="127.0.0.1";//"101.201.47.53";
unsigned short port = 3306;
char *unix_socket = NULL;
unsigned long client_flag = 0;
char table[128] = "plain_tb_test";
MYSQL *conn;
double used_time = 0;
double getUsedTime()
{
    //double f = used_time*1.0/CLOCKS_PER_SEC;
    //printf("Use %ld clocks f = %f\n", used_time, f);
    double f =  used_time/1000;
    return f;
}
int init_db(int size, const char *config_file)
{
    int  i;
    FILE *fp = fopen(config_file, "r");
    int pt = 0;
    if(fp == NULL)
    {
        printf("open myslq config file:%s failed!\n", config_file);
        exit(-1);
    }
    fscanf(fp, "%s%d%s%s%s%s", host, &pt, user, passwd, db, table);
    port = pt;
    fclose(fp);

    sdb = malloc(sizeof(mpz_t)*size);
    if(NULL == sdb)
    {
        printf("init db fialed!\n");
        return -1;
    }
    db_size = size;
    for(i = 0; i < db_size; i++)
        mpz_init(sdb[i]);

    conn = mysql_init(NULL);
    if(NULL==(conn=mysql_real_connect(conn, host, user, passwd, db,
                port, unix_socket, client_flag)))
    {
        printf("connect mysql server failed!\n");
        exit(-1);
    }
    printf("connect mysql success!\n");
    /*printf("Begin init mysql hash data.\n");

    mpz_t v;
    mpz_init(v);
    for(i = 0; i < size; i++)
        getX(i,v);
        */
    return 0;
}
int hash_rows(char *md, char** row, unsigned long *lens, int nrow)
{
    int i;
    SHA_CTX stx;
    SHA_Init(&stx);
    for(i = 0; i < nrow; i++)
        SHA_Update(&stx, row[i], lens[i]);
    SHA_Final(md, &stx);
    /*
    for(i = 0; i < 20; i++)
        printf("%02x", (unsigned char)md[i]);
    printf("\n");
    */
    return 0;
}
int getX(int  x, mpz_t v)
{
    int ret = 0;
    struct timeval tvafter,tvpre;
    struct timezone tz;
    gettimeofday (&tvpre , &tz);
    if(x >= db_size)
    {
        printf("getX: db index outof range! db_size=%d indeX=%d\n", db_size, x);
        exit(-1);
    }
    mpz_set(v, sdb[x]);

    char sql[MAX_SQL_LEN];
    snprintf(sql, MAX_SQL_LEN, "select * from %s where id = %d",
            table, x+1);
    ret = mysql_query(conn, sql/*"select * from plain_db_test limit 1"*/);

    //printf("Query database sql=%s, ret=%d!\n",sql, ret);
    if(ret)
    {
        printf("Query data base failed ret=%d\n", ret);
        exit(-1);
    }

    MYSQL_RES *res = mysql_use_result(conn);
    if(NULL == res)
    {
        printf("mysql get res failed!\n");
        exit(-1);
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if(NULL == row)
    {
        mysql_free_result(res);
        printf("select error, now rows in result.\n");
        exit(-1);
    }
    unsigned long *lens = mysql_fetch_lengths(res);
    //printf("field_count=%d\n", res->field_count);
    if(lens == NULL)
    {
        mysql_free_result(res);
        printf("error:column lengths == NULL!\n");
    }
    int i;
    /*for(i = 0; i < res->field_count; i++)
    {
        printf("column:%d len=%lu\n", i, lens[i]);
    }*/
   // printf("%s %s\n", row[0], row[1]);
    char md[128];
    char md_str[256];
    int len = 0;
    hash_rows(md, row, lens, res->field_count);
    for(i = 0; i < 20 && len < 256; i++, len+=2)
        snprintf(md_str+len, 3, "%02x", (unsigned char)md[i]);
    //printf("SHA1=%s\n", md_str);
    mpz_set_str(v, md_str, 16);
    mysql_free_result(res);
    gettimeofday (&tvafter , &tz);
    used_time += (tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.0;
    return 0;

}

int setX(int  x, mpz_t v)
{
    if(x >= db_size)
    {
        printf("setX:db index outof range! db_size=%d indeX=%d\n", db_size, x);
        return -1;
    }

    mpz_set(sdb[x], v);
   return 0;
}

int updateX(int x)
{
    char buf[255];
    char sql[255];
    int i, ret;
    for(i = 0; i < 5; i++)
        buf[i] = 'a' + rand()%26;
    buf[5] = 0;
    snprintf(sql, 255, "update %s set Family='%s' where id=%d",table,  buf, x+1);
    ret = mysql_query(conn, sql/*"select * from plain_db_test limit 1"*/);
    printf("update Query database ret=%d!\n", ret);
    return ret;
}
void destroy_db()
{
    int  i;
    for(i = 0; i < db_size; i++)
        mpz_clear(sdb[i]);
    free(sdb);
    db_size = 0;
    mysql_close(conn);

}
