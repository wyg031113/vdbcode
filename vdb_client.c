#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
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

int send_hij(int fd, int type, const char *file_name)
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
            break;
        case T_FILE_HIJ:
            return send_hij(fd, T_FILE_HIJ, hij_file);
            break;
        default:
            return -1;
    }
    return -1;
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
        int nc = element_length_in_bytes_compressed(pp->hi[i]);
        max_hi_len = nc > max_hi_len ? nc : max_hi_len;

	}
    if(save_hi(pp->hi, q, max_hi_len) !=0)
        pbc_die("save hi failed!\n");
    pbc_info("save hi to file!\n");
    pbc_info("Compute hi finished!\n");
	element_pp_clear(gpp);

	element_init_Zr(tz, pp->pairing);

	//element_pp_t gpp;

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
	//for(i = 0; i < q; i++)
	//	element_clear(z[i]);
	//free(z);
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

int main()
{
    //int sd = connect_server();
    int q = 100;
    //send_param_file(sd, T_FILE_HIJ);
    //sleep(2);
    //close(sd);
    init_db(q);
    char *gv[3]={"main","param/a.param", NULL};
    vdb_init_save(&ss, q, 2, gv);
    vdb_init_read(&ss, &q, 2, gv);
    return 0;
}
