#ifndef __SIMPLE_DB_H__
#define __SIMPLE_DB_H__
int init_db(int size, const char *config_file);
int getX(int x, mpz_t v);
int setX(int x, mpz_t v);
int updateX(int x);
void destroy_db();
double getUsedTime();
#endif /*__SIMPLE_DB_H__*/
