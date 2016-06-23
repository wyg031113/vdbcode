#include <stdio.h>
#include <string.h>

#include "vdb.h"
#include "simple_db.h"
#include <time.h>
void show_mpz(const char *name, mpz_t v);
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
		element_printf("pp->h[%d]=%B\n", i,  pp->hi[i]);


	printf("Hij is:\n");
	for(i = 0; i < q; i++)
		for(j = i+1; j < q; j++)
			element_printf("h[%d][%d] =%B\n\n", i, j, pp->hij[i*q+j]);

	printf("PK is:\n");
	element_printf("Y=%B\n", ss->PK.Y);
	element_printf("CR=%B\n", ss->PK.CR);
	element_printf("PK.C0=%B\n", ss->PK.C0);
	element_printf("CU0=%B\n", ss->PK.CU0);

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
int hash(element_t H0, element_t Cf1, element_t C0, long long  T)
{
    int len1 = 0;
    int len2 = 0;
    int len3 = 0;
    int len = 0;
    char *buf = NULL;

    len1 = element_length_in_bytes(Cf1);
    len2 = element_length_in_bytes(C0);
    len3 = sizeof(T);
    len += len1;
    len += len2;
    len += len3;

    buf = (char*) malloc(len);
    if(NULL == buf)
    {
        printf("malloc failed in hash!\n");
        exit(-1);
    }
    memset(buf, 0, len);
    element_to_bytes(buf, Cf1);
    element_to_bytes(buf+len1, C0);
    memcpy(buf+len1+len2, &T, len3);
    element_from_hash(H0, buf, len);

    free(buf);
	return 0;
}

inline void getHi(struct pp_struct *p, element_t value, int i)
{
//    element_set(value, p->hi[i]);
	element_pow_zn(value, p->g, p->z[i]);
    //element_printf("p->g=%B z[%d]=%B\n", p->g, i, p->z[i]);
}

inline void getHij(struct pp_struct *p, element_t value, int i, int j)
{
   // element_set(value, p->hij[i*p->q+j]);
    element_t mul;
    element_init_Zr(mul, p->pairing);
    element_mul_zn(mul, p->z[i], p->z[j]);
	element_pow_zn(value, p->g, mul);
    element_clear(mul);
}

inline void setHi(struct pp_struct *p, element_t value, int i)
{
    element_set(p->hi[i], value);
}

inline void setHij(struct pp_struct *p, element_t value, int i, int j)
{
    element_set(p->hij[i*p->q+j], value);
}
int init_Hi(struct pp_struct *p)
{
	p->hi = malloc(sizeof(element_t) * p->q);
	if(NULL == p->hi)
    {
        printf("malloc failed in init_hi.\n");
        exit(-1);
    }
    return 0;
}
int init_Hij(struct pp_struct *p)
{
	p->hij = malloc(sizeof(element_t)*p->q*p->q);
	if(NULL == p->hij)
    {
        printf("malloc failed in init_hi.\n");
        exit(-1);
    }
    return 0;
}
void destroy_Hi(struct pp_struct *p)
{
    free(p->hi);
    p->hi = NULL;
}

void destroy_Hij(struct pp_struct *p)
{
    free(p->hij);
    p->hij = NULL;
}
int vdb_setup(struct setup_struct *ss, int q, int argc, char *argv[])
{
	int i, j;
	element_t *z;
	element_t tz;
	element_t ths;
    element_t hv;
	element_pp_t gpp;
    mpz_t v;
	struct pp_struct *pp;

	memset(ss, 0, sizeof(struct setup_struct));
	ss->T = 0;

	z = malloc(sizeof(element_t) * q);
	if(NULL == z)
		goto out5;


	pp = malloc(sizeof(struct pp_struct));
	if(NULL == pp)
		goto out4;

    pp->q = q;
	//pp->hi = malloc(sizeof(element_t) * q);
	if(0 != init_Hi(pp))
		goto out3;

	//pp->hij = malloc(sizeof(element_t)*q*q);
	if(0 != init_Hij(pp))
		goto out2;


	pbc_demo_pairing_init(pp->pairing, argc, argv); //init G1 G2

	element_init_G1(pp->g, pp->pairing);		//let g be a generator of G1
	element_random(pp->g);

    pbc_info("Beging compute hi...\n");

    pp->z = z;
	element_pp_init(gpp, pp->g);
	for(i = 0; i < q; i++)
	{
		element_init_G1(pp->hi[i], pp->pairing);
		element_init_Zr(z[i], pp->pairing);			//let z in ZZr
		element_random(z[i]);
//		element_pp_pow_zn(pp->hi[i], z[i], gpp);

	}
    pbc_info("Compute hi finished!\n");
	element_pp_clear(gpp);

	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;
    /*
	element_pp_init(gpp, pp->g);
	pbc_info("Begin compute hij\n");
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
    */
	pbc_info("end compute hij\n");

    ss->PK.pp =  ss->S.pp = pp;

	//random chose y==SK
	element_init_Zr(ss->SK, pp->pairing);
	element_random(ss->SK);

	//compute Y
	element_init_G1(ss->PK.Y, pp->pairing);
	element_pow_zn(ss->PK.Y, pp->g, ss->SK); //Y=g^y

	//CR in G1 初始：CR=Cs0=Cf1=
	element_init_G1(ss->PK.CR, pp->pairing);
	element_init_G1(ss->PK.C0, pp->pairing);
	element_init_G1(ss->PK.Cf1, pp->pairing);
	element_init_G1(ss->PK.CU0, pp->pairing);

    mpz_init(v);
    element_init_G1(hv, pp->pairing);
    //element_t hi;
    //element_init_G1(hi, pp->pairing);
    pbc_info("Beging compute init CR.\n");
    clock_t t = clock();
    element_t t1;
    element_t t2;
	element_init_Zr(t1, pp->pairing);
	element_init_Zr(t2, pp->pairing);
    for(i = 0; i < q; i++)
    {
        getX(i, v);
        //getHi(pp, hi, i);

        element_mul_mpz(t1, pp->z[i], v);
        //element_pow_mpz(hv, hi, v);
        if(i == 0)
         //   element_set(ss->PK.CR, hv);
              element_set(t2, t1);
        else
            //element_mul(ss->PK.CR, ss->PK.CR, hv);
            element_add(t2, t2, t1);
    }

    element_pow_zn(ss->PK.CR,  pp->g, t2);
    //printf("t=%lu, now=%lu CPS=%lu diff=%f\n", t, clock(), CLOCKS_PER_SEC, (clock()-t)*1.0/CLOCKS_PER_SEC);
    printf("Beging compute init CR finished, use: %lf seconds!\n", (clock()-t)*1.0/CLOCKS_PER_SEC);
    //element_clear(hi);
    element_clear(t1);
    element_clear(t2);
    mpz_clear(v);
    element_clear(hv);
	element_set(ss->PK.Cf1, ss->PK.CR);
	element_set(ss->PK.CU0, ss->PK.CR);

//############### compute H0 how to?? what is H????
	element_init_G1(ss->H0, pp->pairing);
	element_init_G1(ths, pp->pairing);
	hash(ths, ss->PK.CR, ss->PK.C0, 0);
	element_pow_zn(ss->H0, ths, ss->SK);
	// free z[] and tz
	element_clear(tz);

	//for(i = 0; i < q; i++)
	//	element_clear(z[i]);
	//free(z);
	return 0;
out1:
	free(pp->hij);
out2:
	free(pp->hi);
out3:
	free(pp);
out4:
	//free(z);
out5:
    printf("VDB setup failed!\n");
    exit(-1);
	return -1;
}

/*void getX(mpz_t x, int i)
{
	char buf[20];
	snprintf(buf, 20, "%d", i);
	mpz_set_str(x, buf, 10);
}*/
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
	element_t t1;
	element_t t2;
	element_t t3;
    element_t hij;

	element_init_G1(t1, pp->pairing);
	element_init_Zr(t2, pp->pairing);
	element_init_Zr(t3, pp->pairing);
	element_init_G1(paix, pp->pairing);
	element_init_G1(hij, pp->pairing);
	int j;
	mpz_init(v);
    int first = 0;
    clock_t t = clock();
	for(j = 0; j < q; j++)
		if(j != x)
		{
			getX(j, v);
            //getHij(pp, hij, x, j);
            element_mul_zn(t2, pp->z[x], pp->z[j]);
            element_mul_mpz(t2, t2, v);
		//	element_pow_zn(t1, pp->g, t2);
		//	element_pow_mpz(t1, t1, v);
			//element_pow_mpz(t1, hij, v);
			if(first==0)
            {
				//element_set(paix, t1);
                element_set(t3, t2);
                first = 1;
            }
			else
			{
		//		element_mul(paix, paix, t1);
                element_add(t3, t3, t2);

			}
		}

	element_pow_zn(paix, pp->g, t3);

    printf("prepaired proof pai, used times %f seconds.\n", (clock()-t)*1.0/CLOCKS_PER_SEC);
	mpz_clear(v);
    element_clear(hij);
    element_clear(t1);
    element_clear(t2);
    element_clear(t3);
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
    if(!b1)
    {
    //    pbc_warn("First equation failed!\n");
    }


	//e(GT/HT*hx^vx
	element_init_GT(e3, pp->pairing);
	element_init_GT(e4, pp->pairing);
	element_init_G1(gh, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(ghhv, pp->pairing);
	mpz_init(v);
	getX(x, v);
    //mpz_add(v, v, max_integer);
    element_t hi;
    element_init_G1(hi, pp->pairing);
    getHi(pp, hi, x);
    /*
    if(element_cmp(hi, pp->hi[x]))
    {
        printf("Oh, dear!! x= %d\n", x);
        element_printf("calHi=%B\nhi=%B\n", hi, pp->hi[x]);
        exit(-1);
    }
    */
	element_pow_mpz(hv, hi, v);
	element_div(gh, ss->PK.C0, prf->HT);
	element_div(ghhv, gh, hv);
	pairing_apply(e3, ghhv, hi, pp->pairing);
    element_clear(hi);

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
    if(!b2)
    {
    //    pbc_warn("Second equation failed!\n");
    }
	return b1 && b2;

}

//calculate t'
void show_mpz(const char *name, mpz_t v)
{
    char buf[256];
    mpz_get_str(buf,10,  v);
    printf("%s=%s\n", name, buf);
}
int vdb_update_client(element_t tpx, struct setup_struct *ss, int x, mpz_t vx,  mpz_t new_vx)
{
	element_t ch, hv, hv2,  new_CT, hs, new_HT, hi;
	mpz_t v;

	struct pk_struct *PK = &ss->PK;
	struct s_struct *S = &ss->S;
	struct pp_struct *pp = PK->pp;

	element_init_G1(ch, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(hv2, pp->pairing);
	element_init_G1(new_CT, pp->pairing);
	mpz_init(v);

	element_div(ch, PK->C0, ss->H0); //ch = CT-1/HT-1
    element_init_G1(hi, pp->pairing);
    getHi(pp, hi, x);
	element_pow_mpz(hv, hi, vx); //hv = hx^v
	element_pow_mpz(hv2, hi, new_vx); //hv = hx^v
    element_div(hv, hv, hv2);
	element_div(new_CT, ch, hv);       //CT=ch * hv

    element_clear(hi);
	element_clear(ch);
	element_clear(hv);
	element_clear(hv2);
	mpz_clear(v);

	element_init_G1(hs, pp->pairing);
	element_init_G1(new_HT, pp->pairing);
	//new_T = ss->T + 1;					//T=T+1
	hash(hs, PK->C0, new_CT, ss->T);	//hs=hash(CT-1, CT, T)
	element_pow_zn(new_HT, hs, ss->SK);  //HT = hs^y

	element_set(tpx, new_HT);			//update paramter
	element_set(PK->CU0, new_CT);
	element_set(ss->H0, new_HT);
    element_set(PK->Cf1, PK->C0);
	//ss->T = new_T;

	element_clear(new_CT);
	element_clear(hs);
	element_clear(new_HT);

}

