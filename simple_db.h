#ifndef __SIMPLE_DB_H__
#define __SIMPLE_DB_H__
int init_db(int size);
int getX(int x, mpz_t v);
int setX(int x, mpz_t v);
int updateX(int x);
void destroy_db();
time_t getUsedTime();
#endif /*__SIMPLE_DB_H__*/
