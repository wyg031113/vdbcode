#ifndef __SAVE_PARAM_H__
#define __SAVE_PARAM_H__
#include <pbc/pbc.h>
int save_ele(element_t g, const char *fname);
int read_ele(element_t g, const char *fname);
int save_g(element_t g);
int read_g(element_t g);
int save_hi(element_t *hi, int n, int max_len);
int read_hi(element_t *hi, int n);
int save_hij(element_t *hij, int n, int max_len);
int read_hij(element_t *hij, int n);
int read_int(int *q, const char *fname);
int save_int(int q, const char *fname);
#endif /*__SAVE_PARAM_H__*/
