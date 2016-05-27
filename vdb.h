#ifndef __VDB_H__
#define __VDB_H__

#include <pbc/pbc.h>
#include <pbc/pbc_test.h>
struct pp_struct
{
	element_t p;
	element_t g;
	long long q;
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
    element_t CU0;
	element_t Cf1;
};

struct aux_struct
{
	element_t H0;
	element_t Cf1;
	element_t CU0;
	long long T;
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
	element_t H0;
	long long  T;
};

struct proof_tao
{
	mpz_t vx;
	element_t paix;
	element_t HT;
	element_t CTm1;
	element_t CT;
	long long T;
};

/*pair = |-|(h(x,j) ^ vj)  j in 1 to q and j != x
 *
 */
int vdb_query_paix(element_t paix, struct setup_struct *ss, int x);
int vdb_verify(struct setup_struct *ss, int x, struct proof_tao *prf);
//calculate t'
int vdb_update_client(element_t tpx, struct setup_struct *ss, int x, mpz_t vx,  mpz_t new_vx);
void show_mpz(const char *name, mpz_t v);
#endif /*__VDB_H__*/
