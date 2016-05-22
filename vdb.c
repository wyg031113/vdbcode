#include <stdio.h>
#include <string.h>

#include "vdb.h"

void showss(struct setup_struct *ss)
{
	struct pp_struct *pp = ss->S.pp;
	int q = ss->S.pp->q;
	int i, j;
	printf("q=%d\n", q);
	printf("PP is:\n");
	element_printf("pp->g=%B\n", pp->g);
	printf("Hi is:\n");
	for(i = 0; i < q; i++)	
		element_printf("pp->h[%d]=%B\n", i,  pp->g);


	printf("Hij is:\n");
	for(i = 0; i < q; i++)
		for(j = i+1; j < q; j++)
			element_printf("h[%d][%d] =%B\n\n", i, j, pp->hij[i*q+j]);

	printf("PK is:\n");
	element_printf("Y=%B\n", ss->PK.Y);
	element_printf("CR=%B\n", ss->PK.CR);
	element_printf("C0=%B\n", ss->PK.C0);

	printf("S is: PP aux DB. aux DB\n");
	element_printf("SK=y=%B\n", ss->SK);
	element_printf("H0=%B\n", ss->H0);

}
/* 
 * H0 = (hash(Cf1, C0, T))^y
 * H0,CF1, C0 in G1
 * T is the update times
 * y in ZZp
 */

int hash(element_t H0, element_t Cf1, element_t C0, int T)
{
	element_set(H0, C0);
	return 0;
}	


int vdb_setup(struct setup_struct *ss, int q, int argc, char *argv[])
{
	struct pp_struct *pp;
	element_t *z;
	element_t tz;
	element_t ths;
	int i, j;

	memset(ss, 0, sizeof(struct setup_struct));
	ss->T = 0;

	z = malloc(sizeof(element_t) * q);
	if(NULL == z)
		goto out5;
		

	pp = malloc(sizeof(struct pp_struct));
	if(NULL == pp)
		goto out4;

	pp->hi = malloc(sizeof(element_t) * q);
	if(NULL == pp->hi)
		goto out3;

	pp->hij = malloc(sizeof(element_t)*q*q);
	if(pp->hij == NULL)
		goto out2;

	pp->q = q;

	pbc_demo_pairing_init(pp->pairing, argc, argv); //init G1 G2

	element_init_G1(pp->g, pp->pairing);		//let g be a generator of G1
	element_random(pp->g);

	//compute hi
//	printf("Beging compute hi\n");

	element_pp_t gpp;
	element_pp_init(gpp, pp->g);
	for(i = 0; i < q; i++)	
	{
		//printf("i = %d\n", i);
		element_init_G1(pp->hi[i], pp->pairing);
		element_init_Zr(z[i], pp->pairing);			//let z in ZZr
		element_random(z[i]);
//		element_pow_zn(pp->hi[i], pp->g, z[i]);	
		element_pp_pow_zn(pp->hi[i], z[i], gpp);
	//	element_printf("z=%B\n", z);
	//	element_printf("h%d=%B\n\n", i, pp->hi[i]);
	}

	element_pp_clear(gpp);
	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;
	element_pp_init(gpp, pp->g);
	printf("Begin q*q\n");
	for(i = 0; i < q; i++)
		for(j = i+1; j < q; j++)
		{
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

			element_mul_zn(tz, z[i],z[j]);
			element_pp_pow_zn(pp->hij[i*q+j], tz, gpp);
			element_set(pp->hij[j*q+i], pp->hij[i*q+j]);
		}
	element_pp_clear(gpp);
	printf("end q*q\n");
	/*for(i = 0; i < q; i++)
	{
		element_pp_t gpp;
		element_pp_init(gpp, pp->hi[i]);
		for(j = i+1; j < q; j++)
		{
			
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

			//element_pow_zn(pp->hij[i*q+j], pp->hi[i], z[j]);	
			element_pp_pow_zn(pp->hij[i*q+j], z[j], gpp);
			element_set(pp->hij[j*q+i], pp->hij[i*q+j]);

			//element_printf("h[%d][%d] =%B\n\n", i, j, pp->hij[i*q+j]);
		}
		element_pp_clear(gpp);
	}

*/
	ss->PK.pp =  ss->S.pp = pp;	

	//CR in G1
	element_init_G1(ss->PK.CR, pp->pairing);
	element_init_G1(ss->PK.C0, pp->pairing);
	element_set1(ss->PK.CR);
	element_set(ss->PK.C0, ss->PK.CR);
	
	//random chose y==SK
	element_init_Zr(ss->SK, pp->pairing);
	element_random(ss->SK);

	//compute Y
	element_init_G1(ss->PK.Y, pp->pairing);
	element_pow_zn(ss->PK.Y, pp->g, ss->SK);	
//############### compute H0 how to?? what is H????
	element_init_G1(ss->H0, pp->pairing);
	element_init_G1(ths, pp->pairing);
	hash(ths, ss->PK.CR, ss->PK.C0, 0);
	element_pow_zn(ss->H0, ths, ss->PK.Y);
	// free z[] and tz
	element_clear(tz);
	for(i = 0; i < q; i++)
		element_clear(z[i]);
	free(z);
	return 0;
out1:
	free(pp->hij);
out2:
	free(pp->hi);
out3:
	free(pp);
out4:
	free(z);
out5:
	return -1;
}

void getX(mpz_t x, int i)
{
	char buf[20];
	snprintf(buf, 20, "%d", i);
	mpz_set_str(x, buf, 10);
}
/*pair = |-|(h(x,j) ^ vj)  j in 1 to q and j != x 
 *
 */
int vdb_query_paix(element_t paix, struct setup_struct *ss, int x)
{
	struct pk_struct *PK = &ss->PK;
	struct s_struct *S = &ss->S;
	struct pp_struct *pp = PK->pp;
	int q = pp->q;

	mpz_t v;	
	element_t t1, t2;

	element_init_G1(t1, pp->pairing);
	element_init_G1(t2, pp->pairing);
	element_init_G1(paix, pp->pairing);
	int j;
	mpz_init(v);
	for(j = 0; j < q; j++)
		if(j != x)
		{
			getX(v, j);
			element_pow_mpz(t1, pp->hij[x*q+j], v);	
			if(j==0)
				element_set(paix, t1);
			else
			{
				element_mul(t2, t1, paix);
				element_set(paix, t2);

			}
		}	
	mpz_clear(v);
	return 0;
}

int vdb_verify(struct setup_struct *ss, int x, struct proof_tao *prf)
{
	int b1 = 0, b2 = 0;
	struct pk_struct *PK = &ss->PK;
	struct s_struct *S = &ss->S;
	struct pp_struct *pp = PK->pp;
	mpz_t v;
	element_t e1,e2, e3, e4, hs, gh,hv, ghhv;

	//e(HT,g)
	element_init_GT(e1, pp->pairing);
	element_init_GT(e2, pp->pairing);
	element_init_G1(hs, pp->pairing);
	pairing_apply(e1, prf->HT, pp->g, pp->pairing);
	hash(hs, prf->CTm1, prf->CT, prf->T);
	//e(H(CTm1, CT, T), Y)
	pairing_apply(e2, hs, PK->Y, pp->pairing); 
	//
	b1 = !element_cmp(e1, e2);
	element_clear(e1);
	element_clear(e2);
	element_clear(hs);

	//e(GT/HT*hx^vx
	element_init_GT(e3, pp->pairing);
	element_init_GT(e4, pp->pairing);
	element_init_G1(gh, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(ghhv, pp->pairing);
	mpz_init(v);
	getX(v, x);
	element_pow_mpz(hv, pp->hi[x], v);
	element_div(gh, prf->CT, prf->HT);
	element_mul(ghhv, gh, hv);
	pairing_apply(e3, ghhv, pp->hi[x], pp->pairing); 

	//e(paix, g)
	pairing_apply(e4, prf->paix, pp->g, pp->pairing);
	//
	b2 = !element_cmp(e3, e4);
	element_clear(e3);
	element_clear(e4);
	element_clear(gh);
	element_clear(hv);
	element_clear(ghhv);
	mpz_clear(v);

	return b1 && b2;

}

//after T=T+1
int vdb_update_client(element_t tpx, struct setup_struct *ss, int x, mpz_t vx,  mpz_t new_vx)
{
	element_t ch, hv, new_CT, hs, new_HT;
	mpz_t v; 
	int new_T;

	struct pk_struct *PK = &ss->PK;
	struct s_struct *S = &ss->S;
	struct pp_struct *pp = PK->pp;
	
	element_init_G1(ch, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(new_CT, pp->pairing);
	mpz_init(v);

	element_div(ch, PK->C0, ss->H0);
	mpz_sub(v,  new_vx, vx);
	element_pow_mpz(hv, pp->hi[x], v);
	element_mul(new_CT, ch, hv);
	element_clear(ch);
	element_clear(hv);
	mpz_clear(v);

	element_init_G1(hs, pp->pairing);
	element_init_G1(new_HT, pp->pairing);
	new_T = ss->T + 1;
	hash(hs, PK->C0, new_CT, new_T);
	element_pow_zn(new_HT, hs, PK->Y);
	
	element_set(tpx, new_HT);
	element_set(PK->C0, new_CT);
	element_set(ss->H0, new_HT);
	ss->T = new_T;

	element_clear(new_CT);
	element_clear(hs);
	element_clear(new_HT);
	
}

