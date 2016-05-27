#include <pbc/pbc.h>
#include <stdio.h>
#include "simple_db.h"
static mpz_t *sdb;
static int  db_size;
int init_db(int size)
{
    int  i;
    sdb = malloc(sizeof(mpz_t)*size);
    if(NULL == sdb)
    {
        printf("init db fialed!\n");
        return -1;
    }
    db_size = size;
    for(i = 0; i < db_size; i++)
        mpz_init(sdb[i]);
    return 0;
}

int getX(int  x, mpz_t v)
{
    if(x >= db_size)
    {
        printf("getX: db index outof range! db_size=%d indeX=%d\n", db_size, x);
        return -1;
    }
    mpz_set(v, sdb[x]);
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

void destroy_db()
{
    free(sdb);
    int  i;
    for(i = 0; i < db_size; i++)
        mpz_clear(sdb[i]);
    db_size = 0;

}
