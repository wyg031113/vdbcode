#ifndef __SAVE_PARAM_H__
#define __SAVE_PARAM_H__
#include <pbc/pbc.h>
int save_ele(element_t g, const char *fname);
int read_ele(element_t g, const char *fname);
int save_g(element_t g);
int read_g(element_t g);
int save_arr(element_t *hi, int n, int max_len, const char *fname);
int read_arr(element_t *hi, int n, const char *fname);
int save_hij(element_t *hij, int n, int max_len);
int read_hij(element_t *hij, int n, const char *fname);
int read_int(int *q, const char *fname);
int save_int(int q, const char *fname);
#endif /*__SAVE_PARAM_H__*/
