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

}
int vdb_setup(struct setup_struct *ss, int q, int argc, char *argv[])
{
	struct pp_struct *pp;
	element_t *z;
	element_t tz;
	int i, j;

	memset(ss, 0, sizeof(struct setup_struct));

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
int main(int argc, char *argv[])
{
	struct setup_struct ss;
	vdb_setup(&ss, 1000, argc, argv);
//	showss(&ss);
	return 0;
}
