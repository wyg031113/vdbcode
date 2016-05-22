#ifndef __VDB_H__
#define __VDB_H__

#include <pbc/pbc.h>
#include <pbc/pbc_test.h>
struct pp_struct
{
	element_t p;
	element_t g;
	int q;
	pairing_t pairing;
	element_t *hi;
	element_t *hij;
	//H ???
};

struct pk_struct
{
	struct pp_struct *pp;
	element_t Y;
	element_t CR;
	element_t C0;
};

struct aux_struct
{
	//H0
	element_t Cf1;
	element_t C0;
	int T;
};

struct s_struct
{

	struct pp_struct *pp;
	struct aux_struct *aux;
	//DB///
};

struct setup_struct
{
	struct pk_struct PK;
	struct s_struct S;
	element_t SK;
};


#endif /*__VDB_H__*/
