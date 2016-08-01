#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <getopt.h>
#include "save_param.h"
#include "io.h"
#include "prot.h"
#include "vdb.h"
#include "simple_db.h"

struct setup_struct ss;
struct aux_struct as;
struct proof_tao tao;
#define FILE_NAME_LEN 128
static char serip[17]="127.0.0.1";
static unsigned short port = 7788;
static char hij_file[FILE_NAME_LEN] = "./param/hij";
static char hi_file[FILE_NAME_LEN] = "./param/hi";
static char q_file[FILE_NAME_LEN] = "./param/q";
static char C0_file[FILE_NAME_LEN] = "./param/C0";
static char CU0_file[FILE_NAME_LEN] = "./param/CU0";
static char Cf1_file[FILE_NAME_LEN] = "./param/Cf1";
static char H0_file[FILE_NAME_LEN] = "./param/H0";
static char T_file[FILE_NAME_LEN] = "./param/T";
static char Prog_file[FILE_NAME_LEN] = "./param/Prog";
static char ver_file[FILE_NAME_LEN] = "./param/ver";
static char z_file[FILE_NAME_LEN] = "./param/z";
static char mysql_conf_file[FILE_NAME_LEN] = "./param/mysql_conf";
int connect_server()
{
    int ser = -1;
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = inet_addr(serip);
    if((ser=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket failed in connect server!\n");
        exit(-1);
    }
    if((ser==connect(ser, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr))) < 0)
    {
        printf("Connect server failed!\n");
        exit(-1);
    }

    return ser;
}

int send_header(int fd, int type, int len)
{
    struct packet pkt;
    pkt.type = type;
    pkt.len = len;
    if(write_all(fd, (char *)&pkt, sizeof(pkt)) != sizeof(pkt))
    {
        printf("write header  failed!\n");
        return -1;
    }
    return 0;

}
int send_any(int fd, int type, const char *file_name)
{
    struct packet pkt;
    pkt.type = type;
    pkt.len = get_file_len(file_name);
    if(pkt.len == -1)
    {
        printf("Bad file len.\n");
        return -1;
    }
    if(write_all(fd, (char *)&pkt, sizeof(pkt)) != sizeof(pkt))
    {
        printf("write header hij file failed!\n");
        return -1;
    }

    if(send_file(fd, file_name, pkt.len) != pkt.len)
    {
        printf("write hij file content failed!\n");
        return -1;
    }
    return 0;

}
int send_param_file(int fd, int type)
{

    switch(type)
    {
        case T_FILE_Q:
            return send_any(fd, T_FILE_Q, q_file);
            break;
        case T_FILE_HIJ:
            return send_any(fd, T_FILE_HIJ, hij_file);
            break;

        case T_FILE_H0:
            return send_any(fd, T_FILE_H0, H0_file);
            break;

        case T_FILE_Cf1:
            return send_any(fd, T_FILE_Cf1, Cf1_file);
            break;

        case T_FILE_C0:
            return send_any(fd, T_FILE_C0, C0_file);
            break;

        case T_FILE_CU0:
            return send_any(fd, T_FILE_CU0, CU0_file);
            break;

        case T_FILE_T:
            return send_any(fd, T_FILE_T, T_file);
            break;
        default:
            return -1;
    }
    return -1;
}

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
	//	element_random(z[i]);
        //pbc_info("n=%d\n", n);

	}

    if(read_arr(z, q, z_file) !=0)
        pbc_die("read z failed!\n");
    pbc_info("read z from file!\n");

    if(read_arr(pp->hi, q, hi_file) !=0)
        pbc_die("read hi failed!\n");
    pbc_info("read hi from file!\n");
	element_pp_clear(gpp);

	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;

	/*element_pp_init(gpp, pp->g);
	pbc_info("Begin load hij\n");
	for(i = 0; i < q; i++)
		for(j = i; j < q; j++)
		{
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

		}

    if(read_hij(pp->hij, q, hij_file) !=0)
        pbc_die("read hij failed!\n");
     pbc_info("read hij from file!\n");

	element_pp_clear(gpp);

*/
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

int vdb_init_save(struct setup_struct *ss, int q, int argc, char *argv[])
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
    if(save_g(pp->g) != 0)
    {
        pbc_die("Save g to file failed!\n");
        return -1;
    }
    pbc_info("Select g, and write to file!\n");

    pbc_info("Beging compute hi...\n");

    pp->z = z;
	element_pp_init(gpp, pp->g);

    int max_hi_len = 0;
	for(i = 0; i < q; i++)
	{
		element_init_G1(pp->hi[i], pp->pairing);
        element_init_Zr(z[i], pp->pairing);			//let z in ZZr
		element_random(z[i]);
        //pbc_info("n=%d\n", n);
		element_pp_pow_zn(pp->hi[i], z[i], gpp);
        int n = element_length_in_bytes(pp->hi[i]);
        max_hi_len = n > max_hi_len ? n : max_hi_len;

	}

    if(save_arr(z, q, element_length_in_bytes(z[0]), z_file) !=0)
        pbc_die("save z failed!\n");
    pbc_info("save z to file!\n");
    if(save_arr(pp->hi, q, max_hi_len, hi_file) !=0)
        pbc_die("save hi failed!\n");
    pbc_info("save hi to file!\n");
    pbc_info("Compute hi finished!\n");
	element_pp_clear(gpp);

	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;
/*
	element_pp_init(gpp, pp->g);
	pbc_info("Begin compute hij\n");
    int max_hij_len = 0;
	for(i = 0; i < q; i++)
		for(j = i; j < q; j++)
		{
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

			element_mul_zn(tz, z[i],z[j]);
		    element_pp_pow_zn(pp->hij[i*q+j], tz, gpp);
			element_set(pp->hij[j*q+i], pp->hij[i*q+j]);
            int n = element_length_in_bytes(pp->hij[i*q+j]);
            int nc =  element_length_in_bytes_compressed(pp->hij[i*q+j]);
//          printf("n=%d nc=%d\n", n, nc);
            max_hij_len = nc > max_hij_len ? nc : max_hij_len;

		}

    if(save_hij(pp->hij, q, max_hi_len) !=0)
        pbc_die("save hij failed!\n");
     pbc_info("save hij to file!\n");

	element_pp_clear(gpp);

	pbc_info("end compute hij\n");
*/
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
    element_t hi;
    element_init_G1(hi, pp->pairing);
    pbc_info("Beging compute init CR.\n");
    clock_t t = clock();
    for(i = 0; i < q; i++)
    {
        getX(i, v);
        getHi(pp, hi, i);

        element_pow_mpz(hv, hi, v);
        if(i == 0)
            element_set(ss->PK.CR, hv);
        else
            element_mul(ss->PK.CR, ss->PK.CR, hv);
    }
    //printf("t=%lu, now=%lu CPS=%lu diff=%f\n", t, clock(), CLOCKS_PER_SEC, (clock()-t)*1.0/CLOCKS_PER_SEC);
    printf("Beging compute init CR finished, use: %lf seconds!\n", (clock()-t)*1.0/CLOCKS_PER_SEC);
    element_clear(hi);
    mpz_clear(v);
    element_clear(hv);
	element_set(ss->PK.Cf1, ss->PK.CR);
	element_set(ss->PK.CU0, ss->PK.CR);

//############### compute H0 how to?? what is H????
	element_init_G1(ss->H0, pp->pairing);
	element_init_G1(ths, pp->pairing);
	hash(ths, ss->PK.CR, ss->PK.C0, 0);
	element_pow_zn(ss->H0, ths, ss->SK);

   element_mul(ss->PK.C0, ss->H0, ss->PK.CU0);
	// free z[] and tz
	element_clear(tz);

    //save all
    save_int(q, "param/q");
    save_int(ss->T, "param/T");
    save_ele(ss->H0, "param/H0");
    save_ele(ss->SK, "param/SK");
    save_ele(ss->PK.CR, "param/CR");
    save_ele(ss->PK.C0, "param/C0");
    save_ele(ss->PK.Cf1, "param/Cf1");
    save_ele(ss->PK.CU0, "param/CU0");
    save_ele(ss->PK.Y, "param/Y");
    save_ele(ss->SK, "param/SK");
    save_ele(ss->PK.C0, "param/C0");
	for(i = 0; i < q; i++)
		element_clear(z[i]);
	free(z);
    printf("save finished!\n");
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
void save_prog(int status)
{
    if(save_int(status, Prog_file)!= 0)
    {
        printf("Save status failed!\n");
        exit(-1);
    }

}
int get_prog()
{
    int status = 0;
    if(read_int(&status, Prog_file)!= 0)
    {
        printf("Save status failed!\n");
        save_prog(P_UNINIT);
        return P_UNINIT;
    }
    return status;

}


int init_vdb(int q)
{
    init_db(q, mysql_conf_file);
    char *gv[3]={"main","param/a.param", NULL};
    int ret = P_UNINIT;
    if(get_prog()==P_UNINIT)
    {
        save_prog(P_UNINIT);
        if(vdb_init_save(&ss, q, 2, gv) ==0)
            save_prog(P_INITING);
        else
        {
            printf("vdb init save failed!\n");
            exit(-1);
        }
    }
    int sd = connect_server();
    ret  = P_INITING;
    if(
    //!send_param_file(sd, T_FILE_HIJ) &&
    !send_param_file(sd, T_FILE_T) &&
    !send_param_file(sd, T_FILE_CU0) &&
    !send_param_file(sd, T_FILE_Cf1) &&
    !send_param_file(sd, T_FILE_H0) &&
    !send_param_file(sd, T_FILE_C0) &&
    !send_param_file(sd, T_FILE_Q) &&
    !send_header(sd, T_FINISH, 0) )
        ret = P_FINISH;
    save_prog(ret);
    sleep(2);
    close(sd);
    if(ret == P_FINISH)
        return 0;
    else
        return -1;

}

int  read_vdb(int *q)
{
    char *gv[3]={"main","param/a.param", NULL};
    if(vdb_init_read(&ss, q, 2, gv) == 0)
    {
        return 0;
    }
    return -1;


}


/*server在初始化过程时执行：C0=H0*C(0)
 */
void prep_as(struct setup_struct *ss, struct aux_struct *a)
{
   element_set(a->H0, ss->H0);
   element_set(a->CU0, ss->PK.CU0);
   element_set(a->Cf1, ss->PK.Cf1);
   a->T = ss->T;
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
void prep_proof(struct setup_struct *ss, struct proof_tao *t,
                  struct aux_struct *a, int x)
{
    getX(x, t->vx);
    element_set(t->HT, a->H0);
    element_set(t->CTm1, a->Cf1);
    element_set(t->CT, a->CU0);
    t->T = a->T;
}

void dump_hex(unsigned char *buf, int len)
{
    int i;
    for(i = 0; i < len; i++)
        printf("%02x ", buf[i]);
    printf("\n");
}

int vdb_send_hxj(int fd, int q,  int x)
{
    struct pp_struct *pp = ss.S.pp;
    element_pp_t gpp;
    element_t hxj;
    element_t tz;
    int ret = 0;
    int i,j;
    char buf[256];

    int n = pairing_length_in_bytes_compressed_G1(pp->pairing);

    if(n>256)
        return -1;

    send_header(fd, T_HIJ, n*(x-1));

    element_init_G1(hxj, ss.S.pp->pairing);
    element_init_Zr(tz, ss.S.pp->pairing);
	element_pp_init(gpp, pp->g);
	pbc_info("Begin compute hij\n");
    i = x;
		for(j = 0; j < q; j++)
		{
            if(j == i)
                continue;
			element_mul_zn(tz, pp->z[i], pp->z[j]);
		    element_pp_pow_zn(hxj, tz, gpp);
            memset(buf, 0, n);
            element_to_bytes_compressed(buf, hxj);
            if(write_all(fd, buf, n) != n)
            {
                ret = -1;
                break;
            }
		}

	element_pp_clear(gpp);
    element_clear(hxj);
    element_clear(tz);
    return ret;
}
int vdb_query(int q, int x)
{
    if(x>=q || x < 0)
        return -1;
    if(sizeof(struct packet)!=8)
    {
        printf("sizeof struct pakcet !=8\n");
        exit(-1);
    }
    int sd = connect_server();

    static char buf[1024];


    struct packet *pkt = (struct packet *)buf;
    if(send_header(sd, T_QUERY, sizeof(x)) != 0)
    {
        printf("Send header failed!\n");
        close(sd);
        return -1;
    }
    if(write_all(sd, (char*)&x, sizeof(x)) != sizeof(x))
    {
        printf("Send x failed!\n");
        close(sd);
        return -1;
    }

    /*memset(buf, 0, sizeof(buf));
    int ret = read_all_s(sd, buf, 1024);
    dump_hex(buf, 1024);
*/
    if(read_all(sd, (char*)pkt, sizeof(struct packet))
            != sizeof(struct packet))
    {
        printf("read reply head failed!\n");
        close(sd);
        return -1;
    }
    if(pkt->type != T_REPLY_QUERY)
    {
        printf("Bad reply! type=%x\n", pkt->type);
        close(sd);
        return -1;
    }
    if(1024 - sizeof(struct packet) < pkt->len || pkt->len <=0)
    {
        printf("pkt len too long. failed! pkt_len=%x\n", pkt->len);
        close(sd);
        return -1;
    }
    if(read_all(sd, ((char*)pkt)+sizeof(struct packet), pkt->len)
            != pkt->len)
    {
        printf("real pai failed!\n");
        close(sd);
        return -1;
    }
    element_t pai;
    element_init_G1(pai, ss.S.pp->pairing);
    if(element_from_bytes(pai, ((char*)pkt+sizeof(struct packet))) <= 0)
    {
        printf("parse pai failed!\n");
        close(sd);
        return -1;
    }
    close(sd);
    init_as_tao(&ss, &as, &tao);
    ss.S.aux = &as;
    prep_as(&ss, &as);
    prep_proof(&ss, &tao, &as, x);
    element_set(tao.paix, pai);
    //show_proof(&tao);
    //char tbuf[232];
    //element_to_bytes(tbuf, tao.CTm1);
    int b = vdb_verify(&ss, x,  &tao);
    if(!b)
        printf("equal:Verify failed!\n");
    destroy_as_tao(&as, &tao);
    return b;

}

int init(int q)
{
    save_prog(P_UNINIT);
    if(init_vdb(q) != 0)
    {
        printf("init_vdb failed!\n");
        return -1;
    }
    return 0;
}
int query(int x)
{
    int q = 0;
    save_int(Q_ING, ver_file);
    read_vdb(&q);
    printf("Database size:%d rows.\n", q);
    init_db(q, mysql_conf_file);
    if(vdb_query(q, x) == 1)
    {
        save_int(Q_SUCCESS, ver_file);
        printf("Verify success!\n");
        return 1;
    }
    else
    {
        save_int(Q_FAILED, ver_file);
        printf("Verify failed!\n");
        return -2;
    }

}
int main(int argc, char *argv[])
{
    int q = -1;
    int be_init = 0;
    int be_query = 0;
    int x = -1;
    int opt;
    while((opt = getopt(argc, argv, "in:qx:s:p:")) != -1)
    {
        switch(opt){
            case 'i':
                be_init = 1;
                break;
            case 'n':
                q = atoi(optarg);
                break;
            case 'q':
                be_query = 1;
                break;
            case 'x':
                x = atoi(optarg);
                break;
            case 's':
                strncpy(serip, optarg, 17);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr,"Usage: vdb_client [-i -n 100] [-q -x 23]\n");
                break;
        }
    }
    if((be_init == 1 && q <= 0) || (be_init==0&&q>0))
    {
        printf("-i -n must be coexist.\n");
        return -1;
    }


    if((be_query == 1 && x < 0) || (be_query==0 && x>=0))
    {
        printf("-q -x  must be coexist.\n");
        return -1;
    }
    if(be_init)
        return init(q);
    if(be_query)
        return query(x);
    return 0;
}
