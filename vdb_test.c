/*VDB测试程序 参考
 * 作者:王永刚
 * 2016.5.27
 * email:wyg_0802@126.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vdb.h"

int q = 100;
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
    printf("T=%lld\n", t->T);
}

/*模拟server执行query的过程
 */
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

/*模拟客户端验证的过程
 */
int test_verify(int x)
{
   // printf("Test verify.\n");
    int b = vdb_verify(&ss, x,  &tao);
    //printf("verify ret:%d\n", b);
    return b;
}

/*模拟server端进行更新的过程
 */
int server_update(struct setup_struct *ss, element_t tx,
                    struct aux_struct *a, mpz_t v,  mpz_t vt, int x)
{
    int ret = 0;
    element_t tpx;
    element_t CT;
    //mpz_t v;
    //mpz_init(v);
    //getX(x, v);
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
    //mpz_clear(v);
    return ret;

}

/*客户端发起更新
 */
void test_client_update(int x, mpz_t v, mpz_t vt)
{
    element_t tpx;
    element_init_G1(tpx, ss.PK.pp->pairing);
    ss.T++;
   // element_printf("Before update:v=%B\nVt=%B\n", v, vt);
    vdb_update_client(tpx, &ss, x, v, vt);
    server_update(&ss, tpx, &as, v, vt, x);
    element_clear(tpx);
}

/*查询db[x]
 */
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


}
#define MAX_LEN 60
void random_vx(mpz_t vx)
{
    char buf[MAX_LEN+1];
    int len = (rand() % MAX_LEN) + 1;
    int i;
    for(i = 0; i < len; i++)
        buf[i] = rand()%10 + '0';
    buf[len] = 0;
    mpz_set_str(vx, buf, 10);
}

/*模拟初始话过程
 */
int vdb_init_read(struct setup_struct *ss,int *tq,  int argc, char *argv[]);
void test_setup_vdb(int argc, char *argv[])
{
   // init_db(q);
    pbc_info("begin Setup...\n");
	//vdb_setup(&ss, q, argc, argv);
    char *gv[3]={"main","param/a.param", NULL};

    vdb_init_read(&ss, &q, 2, gv);
    init_db(q);

    init_as_tao(&ss, &as, &tao);
    server_setup(&ss, &as);  //client send H0 to server, server:C0=H0*C0
	//showss(&ss);
    ss.S.aux = &as;
    pbc_info("setup finished!\n");

}

void show_db_data()
{
    int i;
    mpz_t v;

    mpz_init(v);
    for(i = 0; i < q; i++)
    {
        getX(i, v);
        printf("%d.", i);
        show_mpz("v=", v);
    }
}

//----------------------------------------------------------
int vdb_init_read(struct setup_struct *ss,int *tq,  int argc, char *argv[])
{
	int i, j, q = 0;
	element_t *z;
	element_t tz;
	element_t ths;
    element_t hv;
	element_pp_t gpp;
    mpz_t v;
	struct pp_struct *pp;
    read_int(&q, "param/q");
    *tq = q;
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
    if(read_g(pp->g) != 0)
    {
        pbc_die("read g to file failed!\n");
        return -1;
    }
    pbc_info("read g from file!\n");

    pbc_info("Beging compute hi...\n");

    pp->z = z;
	element_pp_init(gpp, pp->g);

	for(i = 0; i < q; i++)
	{
		element_init_G1(pp->hi[i], pp->pairing);
        element_init_Zr(z[i], pp->pairing);			//let z in ZZr
		element_random(z[i]);
        //pbc_info("n=%d\n", n);

	}
    if(read_hi(pp->hi, q) !=0)
        pbc_die("read hi failed!\n");
    pbc_info("read hi from file!\n");
	element_pp_clear(gpp);

	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;

	element_pp_init(gpp, pp->g);
	pbc_info("Begin load hij\n");
	for(i = 0; i < q; i++)
		for(j = i; j < q; j++)
		{
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

		}

    if(read_hij(pp->hij, q) !=0)
        pbc_die("read hij failed!\n");
     pbc_info("read hij from file!\n");

	element_pp_clear(gpp);


    ss->PK.pp =  ss->S.pp = pp;

	//random chose y==SK
	element_init_Zr(ss->SK, pp->pairing);

	//compute Y
	element_init_G1(ss->PK.Y, pp->pairing);

	//CR in G1 初始：CR=Cs0=Cf1=
	element_init_G1(ss->PK.CR, pp->pairing);
	element_init_G1(ss->PK.C0, pp->pairing);
	element_init_G1(ss->PK.Cf1, pp->pairing);
	element_init_G1(ss->PK.CU0, pp->pairing);


//############### compute H0 how to?? what is H????
	element_init_G1(ss->H0, pp->pairing);
	element_init_G1(ths, pp->pairing);

    //save all
    read_int(&(ss->T), "param/T");
    read_ele(ss->H0, "param/H0");
    read_ele(ss->SK, "param/SK");
    read_ele(ss->PK.CR, "param/CR");
    read_ele(ss->PK.C0, "param/C0");
    read_ele(ss->PK.Cf1, "param/Cf1");
    read_ele(ss->PK.CU0, "param/CU0");
    read_ele(ss->PK.Y, "param/Y");
    read_ele(ss->SK, "param/SK");
	//for(i = 0; i < q; i++)
	//	element_clear(z[i]);
	//free(z);
    printf("read finished!\n");
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
	return -1;
}
//------------------------------------------------------------
void test_main()
{
    int n = 100;
    int i;
    int ret = 0;
    srand(time(0));
    mpz_t v, vx, vt;

    mpz_init(v);
    mpz_init(vx);
    mpz_init(vt);
    time_t t1, t2, t3, t4;
    double t10, t11;
    pbc_info("test begin: test case:%d q=%d\n", n, q);
    for(i = 0; i < n; i++)
    {
        int x = rand()%q;

        pbc_info("test %d: x=%d\n", i+1, x);

        pbc_info("Query and verify test begin....\n");
        t1 = time(NULL);
        t10 = getUsedTime();
        test_query(v,x);
        ret = test_verify(x);
        t11 = getUsedTime();
        printf("t10=%f t11=%f\n", t10, t11);
        if(!ret)
        {
            pbc_info("qeury verify failed!:test=:%d x=%d\n", i, x);
            exit(-1);
        }
        t2 = time(NULL);
        printf("Query and verify OK. use %lu seconds.\n", t2-t1);

        pbc_info("Update query and verify test begin....\n");
        //random_vx(vx);
        updateX(x);
        getX(x, vx);
        test_client_update(x, v, vx);
        t3 = time(NULL);
        pbc_info("update OK. use %lu seconds\n", t3-t2);

        pbc_info("verify after update...\n");
        test_query(v,x);
        //updateX((x+1)%q);
        ret = test_verify(x);
        if(!ret)
        {
            pbc_info("qeury verify after update failed!:test=%d x=%d\n", i, x);
            exit(-1);
        }
        t4 = time(NULL);
        pbc_info("query update query and verify OK. used time:%lu seconds.\n", t4 - t3);
/*
        //random_vx(vx);
        updateX(x);
        getX(x, vx);
        test_client_update(x, v, vx);
        random_vx(vt);
        setX(x, vt);
        test_query(v,x);
        ret = test_verify(x);
        if(ret)
        {
            pbc_info("qeury verify after server change db, must be failed!:test=:%d x=%d\n", i, x);
            exit(-1);
        }
        pbc_info("update modify query and verify OK\n");

        setX(x, vx);
        test_query(v,x);
        ret = test_verify(x);
        if(!ret)
        {
            pbc_info("qeury verify after recovery db failed!:test=:%d x=%d\n", i, x);
            exit(-1);
        }
        pbc_info("recovery query and verify OK\n");

        int t = rand()%q;
        setX(x, vx);
        test_query(v,x);
        ret = test_verify(x);
        if(!ret)
        {
            pbc_info("qeury verify after recovery db failed!:test=:%d x=%d\n", i, x);
            exit(-1);
        }
        pbc_info("recovery query and verify OK\n");
*/
        pbc_info("test %d/%d finished!\n\n", i+1, n);
    }

    sleep(1);
    printf("###################################################\n");
    show_db_data();
    mpz_clear(v);
    mpz_clear(vx);
    mpz_clear(vt);
}
int main(int argc, char *argv[])
{
    time_t t1 = time(NULL);
       test_setup_vdb(argc, argv);
    time_t t2 = time(NULL);
    printf("setup use time: %lu seconds\n", t2-t1);
    test_main();
    return 0;
    init_db(q);
    updateX(2);
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
