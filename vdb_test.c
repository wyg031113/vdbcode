#include <stdio.h>
#include "vdb.h"
int q = 10;
void test_vdb()
{
    int i;
    char s[256];
    char vs[256];
    mpz_t u,v;
    mpz_init(v);
    mpz_init(u);
    for(i = 0; i < q; i++)
    {
        snprintf(vs, 256, "123456789%d", i);
        mpz_set_str(v, vs, 10);
        setX(i, v);

        getX(i, u);
        mpz_get_str(s, 10, u);
        printf("u=%s\n", s);
    }

}

struct setup_struct ss;
struct aux_struct as;
struct proof_tao tao;
void init_as_tao(struct setup_struct *s,
                 struct aux_struct *a,
                 struct proof_tao *t)
{
    struct pp_struct *pp = s->PK.pp;
    element_init_G1(a->H0, pp->pairing);
    element_init_G1(a->Cf1, pp->pairing);
    element_init_G1(a->C0, pp->pairing);
    a->T = 0;

    mpz_init(t->vx);
    element_init_G1(t->paix, pp->pairing);
    element_init_G1(t->HT, pp->pairing);
    element_init_G1(t->CTm1, pp->pairing);
    element_init_G1(t->CT, pp->pairing);
    t->T  = 0;
}
void destroy_as_tao(struct aux_struct *a,
                    struct proof_tao *t)
{
    element_clear(a->H0);
    element_clear(a->Cf1);
    element_clear(a->C0);

    mpz_clear(t->vx);
    element_clear(t->paix);
    element_clear(t->HT);
    element_clear(t->CTm1);
    element_clear(t->CT);
}

void server_setup(struct setup_struct *ss, struct aux_struct *a)
{
   element_set(a->H0, ss->H0);
   element_set(a->C0, ss->PK.C0);
   element_set(a->Cf1, ss->PK.C0);
   a->T = 0;
   element_mul(a->H0, a->H0, a->C0);

}

void server_query(struct setup_struct *ss, struct proof_tao *t,
                  struct aux_struct *a, int x)
{
    getX(x, t->vx);
    element_set(t->HT, a->H0);
    element_set(t->CTm1, a->Cf1);
    element_set(t->CT, a->C0);
    t->T = a->T;
    vdb_query_paix(t->paix, ss, x);
}

void test_verify(int x)
{
    printf("Test verify.\n");
    int b = vdb_verify(&ss, x,  &tao);
    printf("verify ret:%d\n", b);
}

int server_update(struct setup_struct *ss, element_t tx,
                    struct aux_struct *a,  mpz_t vt, int x)
{
    int ret = 0;
    element_t tpx;
    element_t CT;
    mpz_t v;
    mpz_init(v);
    getX(x, v);
    element_init_G1(tpx, ss->PK.pp->pairing);
    element_init_G1(CT, ss->PK.pp->pairing);
    vdb_update_client(tpx, ss, 0, v, vt);
    element_printf("server:tpx = %B\n", tpx);

    if(element_cmp(tpx, tx))
    {
        printf("check tpx failed!!\n");
        ret = -1;
        goto out;
    }
    setX(x, vt);
    element_mul(CT, tx, a->C0);
    element_set(a->Cf1, a->C0);
    element_set(a->C0, CT);
    element_set(a->H0, tx);
    a->T++;
out:
    element_clear(tpx);
    element_clear(CT);
    mpz_clear(v);
    return ret;

}
void test_client_update(int x, mpz_t v, mpz_t vt)
{
    element_t tpx;
    element_init_G1(tpx, ss.PK.pp->pairing);
    ss.T++;
    vdb_update_client(tpx, &ss, 0, v, vt);
    element_printf("tpx = %B\n", tpx);
    server_update(&ss, tpx, &as, vt, x);
    element_clear(tpx);
}
void test_query(mpz_t v, int x)
{

    printf("test query x = %d\n", x);
    server_query(&ss, &tao, &as, x);
    char buf[256];
    mpz_get_str(buf, 10, tao.vx);
    printf("db[x]=%s\n", buf);

    test_verify(x);

}
int main(int argc, char *argv[])
{
    init_db(q);
	printf("vdb client running....\n");

    printf("Setup...\n");
	vdb_setup(&ss, q, argc, argv);
    init_as_tao(&ss, &as, &tao);
	showss(&ss);
    server_setup(&ss, &as);  //client send H0 to server, server:C0=H0*C0
    ss.S.aux = &as;


    mpz_t v, vt;
    mpz_init(v);
    mpz_init(vt);
    test_query(v, 0);

    printf("test update.\n");
    mpz_set_str(vt, "12345", 10);
    test_client_update(0, v, vt);

    printf("+++++++++++++++++++++++\n");
    test_query(v, 0);

/*    printf("test update.\n");
    mpz_set_str(vt, "9999999", 10);
    test_client_update(0, v, vt);
*/

    mpz_clear(v);
    mpz_clear(vt);
    destroy_as_tao(&as, &tao);
    destroy_db();
	return 0;
}
