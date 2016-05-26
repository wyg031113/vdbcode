#ifndef __SIMPLE_DB_H__
#define __SIMPLE_DB_H__
int init_db(int size);
int getX(int x, mpz_t v);
int setX(int x, mpz_t v);
void destroy_db();
#endif /*__SIMPLE_DB_H__*/
