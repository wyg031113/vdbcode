#include <stdio.h>
#include <string.h>

#include "vdb.h"
#include "simple_db.h"
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
int hash(element_t H0, element_t Cf1, element_t C0, int T)
{
    int len1 = 0;
    int len2 = 0;
    int len3 = 0;
    int len = 0;
    char *buf = NULL;
    //mpz_t t;
    //element_t  he;

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
    /*mpz_init2(t, T);
    element_init_same_as(he, C0);
    printf("T=%d\n", T);
    element_add(he, C0, ht);
	element_mul_mpz(H0, ht, t);
    element_add(H0, H0, ht);
    element_add(H0, H0, Cf1);
    //element_printf("H0=%B\n", H0);
    mpz_clear(t);
    element_clear(he);
    */
    element_printf("Hash:%B\n", H0);
    free(buf);
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

   // element_init_G1(ht, pp->pairing);            //init a ranodm value for hash
    //element_random(ht);
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
	printf("Begin compute hij\n");
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
	printf("end compute hij\n");

    ss->PK.pp =  ss->S.pp = pp;

	//random chose y==SK
	element_init_Zr(ss->SK, pp->pairing);
	element_random(ss->SK);
    element_printf("SK=%B\n", ss->SK);

	//compute Y
	element_init_G1(ss->PK.Y, pp->pairing);
	element_pow_zn(ss->PK.Y, pp->g, ss->SK); //Y=g^y
    element_printf("Y=%B\n", ss->PK.Y);

	//CR in G1 初始：CR=Cs0=Cf1=
	element_init_G1(ss->PK.CR, pp->pairing);
	element_init_G1(ss->PK.C0, pp->pairing);
	element_init_G1(ss->PK.Cf1, pp->pairing);
	element_init_G1(ss->PK.CU0, pp->pairing);
    mpz_t v;
    mpz_init(v);
    element_t hv;
    element_init_G1(hv, pp->pairing);
	element_set1(ss->PK.CR);
    for(i = 0; i < q; i++)
    {
        getX(i, v);
        element_pow_mpz(hv, pp->hi[i], v);
        if(i == 0)
            element_set(ss->PK.CR, hv);
        else
            element_mul(ss->PK.CR, ss->PK.CR, hv);
    }
    element_clear(hv);
    element_printf("CR====%B\n", ss->PK.CR);
	element_set(ss->PK.Cf1, ss->PK.CR);
	element_set(ss->PK.CU0, ss->PK.CR);

//############### compute H0 how to?? what is H????
	element_init_G1(ss->H0, pp->pairing);
	element_init_G1(ths, pp->pairing);
	hash(ths, ss->PK.CR, ss->PK.C0, 0);
    element_printf("tsh=%B ", ths);
	element_pow_zn(ss->H0, ths, ss->SK);
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

	element_init_G1(t1, pp->pairing);
	element_init_G1(paix, pp->pairing);
	int j;
	mpz_init(v);
    int first = 0;
	for(j = 0; j < q; j++)
		if(j != x)
		{
			getX(j, v);
			element_pow_mpz(t1, pp->hij[x*q+j], v);
			if(first==0)
            {
				element_set(paix, t1);

                first = 1;
            }
			else
			{
				element_mul(paix, paix, t1);

			}
            printf("j=%d  ", j);
            element_printf("--paix=%B hij=%B\n", paix, pp->hij[x*q+j]);
		}
	mpz_clear(v);
    element_clear(t1);
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
        printf("First equation failed!\n");


	//e(GT/HT*hx^vx
	element_init_GT(e3, pp->pairing);
	element_init_GT(e4, pp->pairing);
	element_init_G1(gh, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(ghhv, pp->pairing);
	mpz_init(v);
	getX(x, v);
    //mpz_add(v, v, max_integer);
	element_pow_mpz(hv, pp->hi[x], v);
    element_printf("hv=%B\n", hv);
    element_printf("hi=%B\n", pp->hi[x]);
    printf("x=%d\n", x);
    show_mpz("xxxv", v);
	element_div(gh, ss->PK.C0, prf->HT);
    element_printf("CT/HT=%B\n", gh);
	element_div(ghhv, gh, hv);
    element_printf("ghhv=%B\n", ghhv);
	pairing_apply(e3, ghhv, pp->hi[x], pp->pairing);

	//e(paix, g)
	pairing_apply(e4, prf->paix, pp->g, pp->pairing);
	//
    element_printf("e3=%B\ne4=%B\n", e3, e4);
	b2 = !element_cmp(e3, e4);
	element_clear(e3);
	element_clear(e4);
	element_clear(gh);
	element_clear(hv);
	element_clear(ghhv);
	mpz_clear(v);
    if(!b2)
        printf("Second equation failed!\n");
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
	element_t ch, hv, hv2,  new_CT, hs, new_HT;
	mpz_t v,zero;

	struct pk_struct *PK = &ss->PK;
	struct s_struct *S = &ss->S;
	struct pp_struct *pp = PK->pp;

	element_init_G1(ch, pp->pairing);
	element_init_G1(hv, pp->pairing);
	element_init_G1(hv2, pp->pairing);
	element_init_G1(new_CT, pp->pairing);
	mpz_init(v);
	//mpz_init(zero);
    show_mpz("vx", vx);
    show_mpz("new_vx", new_vx);
	element_div(ch, PK->C0, ss->H0); //ch = CT-1/HT-1
    element_printf("======ch=%B\n", ch);
	//mpz_sub(v,  vx, new_vx);		 //v = v' - v
    //mpz_add(v, v, max_integer);
    //mpz_mod(v, v, max_integer);
	//mpz_sub(v,  zero, v);		 //v = v' - v
//    show_mpz("v-v'", v);
   // element_printf("======v'-v=%B\n", ch);
	element_pow_mpz(hv, pp->hi[x], vx); //hv = hx^v
	element_pow_mpz(hv2, pp->hi[x], new_vx); //hv = hx^v
    element_div(hv, hv, hv2);

    element_t tmp;
    element_init_G1(tmp, pp->pairing);
    element_pow_mpz(tmp, pp->hi[x], new_vx);
    element_mul(tmp, tmp, hv);
    element_printf("tmp=%B\n", tmp);
    show_mpz("vx", vx);
	element_pow_mpz(tmp, pp->hi[x], vx); //hv = hx^v
    element_printf("tmp2=%B\n", tmp);
    element_printf("======hv=%B\n", hv);
    printf("x=%d\n", x);
    element_printf("======hi=%B\n", pp->hi[x]);
	element_div(new_CT, ch, hv);       //CT=ch * hv
	element_clear(ch);
	element_clear(hv);
	element_clear(hv2);
	mpz_clear(v);

	element_init_G1(hs, pp->pairing);
	element_init_G1(new_HT, pp->pairing);
	//new_T = ss->T + 1;					//T=T+1
    element_printf("########before hash: CT-1:%B\nCUT:%B\n", PK->C0, new_CT);
    printf("T=%d\n", ss->T);
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

