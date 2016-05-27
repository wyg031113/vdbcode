#include <stdio.h>
#include "vdb.h"
int q = 10;
void test_vdb() //虚拟数据库测试
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
    mpz_clear(v);
    mpz_clear(u);
}

struct setup_struct ss;
struct aux_struct as;
struct proof_tao tao;
/*初始化证据tao和附加信息aux
 */
void init_as_tao(struct setup_struct *s,
                 struct aux_struct *a,
                 struct proof_tao *t)
{
    struct pp_struct *pp = s->PK.pp;
    element_init_G1(a->H0, pp->pairing);
    element_init_G1(a->Cf1, pp->pairing);
    element_init_G1(a->CU0, pp->pairing);
    a->T = 0;

    mpz_init(t->vx);
    element_init_G1(t->paix, pp->pairing);
    element_init_G1(t->HT, pp->pairing);
    element_init_G1(t->CTm1, pp->pairing);
    element_init_G1(t->CT, pp->pairing);
    t->T  = 0;
}

/*销毁证据tao和附加信息aux
 */
void destroy_as_tao(struct aux_struct *a,
                    struct proof_tao *t)
{
    element_clear(a->H0);
    element_clear(a->Cf1);
    element_clear(a->CU0);

    mpz_clear(t->vx);
    element_clear(t->paix);
    element_clear(t->HT);
    element_clear(t->CTm1);
    element_clear(t->CT);
}

/*server在初始化过程时执行：C0=H0*C(0)
 */
void server_setup(struct setup_struct *ss, struct aux_struct *a)
{
   element_set(a->H0, ss->H0);
   element_set(a->CU0, ss->PK.CU0);
   element_set(a->Cf1, ss->PK.Cf1);
   a->T = 0;
   element_mul(ss->PK.C0, a->H0, ss->PK.CU0);
}

void show_proof(struct proof_tao *t)
{
    char buf[256];
    mpz_get_str(buf, 10, t->vx);
    printf("--------------show proof-------------------\n");
    printf("db[x]=%s\n", buf);
    element_printf("Paix=%B\nHT=%B\nCT-1=%B\nCT=%B\n",
                    t->paix, t->HT, t->CTm1, t->CT);
    printf("T=%d\n", t->T);
}
void server_query(struct setup_struct *ss, struct proof_tao *t,
                  struct aux_struct *a, int x)
{
    getX(x, t->vx);
    element_set(t->HT, a->H0);
    element_set(t->CTm1, a->Cf1);
    element_set(t->CT, a->CU0);
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
   /* vdb_update_client(tpx, ss, x, v, vt);
    element_printf("server:tpx = %B\n", tpx);

    if(element_cmp(tpx, tx))
    {
        printf("check tpx failed!!\n");
        ret = -1;
        goto out;
    }
    */
    setX(x, vt);
    element_mul(CT, ss->H0, ss->PK.CU0);
    element_set(a->CU0, ss->PK.CU0);
    element_set(a->Cf1, ss->PK.Cf1);
    element_set(ss->PK.C0, CT);
    element_set(a->H0, tx);
    a->T = ss->T;
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
   // element_printf("Before update:v=%B\nVt=%B\n", v, vt);
    vdb_update_client(tpx, &ss, x, v, vt);
    server_update(&ss, tpx, &as, vt, x);
    element_clear(tpx);
}
void test_query(mpz_t v, int x)
{

    server_query(&ss, &tao, &as, x);
    mpz_set(v, tao.vx);
    /*char buf[256];
    mpz_get_str(buf, 10, tao.vx);
    printf("db[x]=%s\n", buf);
    */

    mpz_t iv;
    mpz_init(iv);
    mpz_set_str(iv, "87777", 10);
    //setX(1, iv);
    mpz_clear(iv);

    test_verify(x);

}
int main(int argc, char *argv[])
{
    init_db(q);
    /*mpz_t iv;
    mpz_init(iv);
    mpz_set_str(iv, "77777", 10);
    setX(0, iv);

    mpz_set_str(iv, "9999", 10);
    setX(1, iv);

    mpz_set_str(iv, "4424242342253", 10);
    setX(2, iv);

    mpz_set_str(iv, "4424", 10);
    setX(3, iv);

    //mpz_set_str(iv, "4424", 10);
    //setX(4, iv);

    mpz_clear(iv);
*/


    printf("begin Setup...\n");
	vdb_setup(&ss, q, argc, argv);
    init_as_tao(&ss, &as, &tao);
    server_setup(&ss, &as);  //client send H0 to server, server:C0=H0*C0
	//showss(&ss);
    ss.S.aux = &as;


    mpz_t v, vt;
    mpz_init(v);
    mpz_init(vt);
    /*test_query(v, 0);
    //test_query(v, 0);

    printf("ok,before update\n");
    fflush(stdin);
    //element_printf("test update. v = %B\n", v);
    mpz_set_str(vt, "12345", 10);
    test_client_update(0, v, vt);
*/

    pbc_info("setup finished!\n");
    /*test_query(v, 0);


    mpz_set_str(vt, "252", 10);
    test_client_update(0, v, vt);
    test_query(v, 0);

   printf("test update.\n");
    test_query(v,1);
    mpz_set_str(vt, "9865", 10);
    test_client_update(1, v, vt);
    test_query(v,1);
*/
/*printf("test update.\n");
    test_query(v,2);
    mpz_set_str(vt, "65010", 10);
    test_client_update(2, v, vt);
    test_query(v,2);
*/
printf("test update.\n");
    test_query(v,3);
    mpz_set_str(vt, "620000", 10);
    test_client_update(3, v, vt);
    test_query(v,3);


printf("test update.\n");
    test_query(v,8);
    mpz_set_str(vt, "1231487", 10);
    test_client_update(8, v, vt);
    test_query(v,8);



    mpz_clear(v);
    mpz_clear(vt);
    destroy_as_tao(&as, &tao);
    destroy_db();
	return 0;
}
