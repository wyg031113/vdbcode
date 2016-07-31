#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <pbc/pbc.h>

#include "vdb.h"
#include "io.h"
#include "prot.h"
#define FILE_NAME_LEN 128
static unsigned short port = 7788;
static char q_file[FILE_NAME_LEN]="./sparam/q";
static char hij_file[FILE_NAME_LEN]="./sparam/hij";
static char C0_file[FILE_NAME_LEN] = "./sparam/C0";
static char CU0_file[FILE_NAME_LEN] = "./sparam/CU0";
static char Cf1_file[FILE_NAME_LEN] = "./sparam/Cf1";
static char H0_file[FILE_NAME_LEN] = "./sparam/H0";
static char T_file[FILE_NAME_LEN] = "./sparam/T";
struct setup_struct ss;
struct aux_struct as;
struct proof_tao tao;
static int q = 0;
int vdb_init_read(struct setup_struct *ss,int *tq,  int argc, char *argv[])
{
	int i, j, q = 0;
//	element_t *z;
	element_t tz;
	element_t ths;
    element_t hv;
	element_pp_t gpp;
    mpz_t v;
	struct pp_struct *pp;
    read_int(&q, q_file);
    *tq = q;
	memset(ss, 0, sizeof(struct setup_struct));
	ss->T = 0;

	/*z = malloc(sizeof(element_t) * q);
	if(NULL == z)
		goto out5;
*/

	pp = malloc(sizeof(struct pp_struct));
	if(NULL == pp)
		goto out4;

    pp->q = q;
	//pp->hi = malloc(sizeof(element_t) * q);
/*	if(0 != init_Hi(pp))
		goto out3;
*/
	//pp->hij = malloc(sizeof(element_t)*q*q);
	if(0 != init_Hij(pp))
		goto out2;


//	pbc_demo_pairing_init(pp->pairing, argc, argv); //init G1 G2

	/*element_init_G1(pp->g, pp->pairing);		//let g be a generator of G1
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
*/
	//element_pp_t gpp;

	element_pp_init(gpp, pp->g);
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
    read_int(&(ss->T), T_file);
    read_ele(ss->H0, H0_file);
    //read_ele(ss->SK, "param/SK");
    //read_ele(ss->PK.CR, "param/CR");
    read_ele(ss->PK.C0, C0_file);
    read_ele(ss->PK.Cf1, Cf1_file);
    read_ele(ss->PK.CU0, CU0_file);
   // read_ele(ss->PK.Y, "param/Y");
    //read_ele(ss->SK, "param/SK");
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
int start_listen(void)
{
    int listen_fd = -1;
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    if((listen_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket failed!\n");
        exit(-1);
    }

    if(bind(listen_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr))!=0)
    {
        printf("Bind failed!\n");
        exit(-1);
    }
    if(listen(listen_fd, 5) != 0)
    {
        printf("Listen failed!\n");
        exit(-1);
    }
    return listen_fd;

}
#define BUF_SIZE 2048
static char rcv_buf[BUF_SIZE];

int be_reload = 1;
/**
 * pkt len is BUF_SIZE
 */
int handle_pkt(int fd, struct packet *pkt)
{
    int ret = 0;
    switch(pkt->type)
    {
        case T_FILE_Q:
            printf("Receive file q.\n");
            return recv_file(fd, q_file, pkt->len) == pkt->len;
            break;
        case T_FILE_HIJ:
            printf("Receive file hij.\n");
            return recv_file(fd, hij_file, pkt->len) == pkt->len;
            break;
        case T_FILE_H0:
            return recv_file(fd, H0_file, pkt->len)== pkt->len;
            break;

        case T_FILE_Cf1:
            return recv_file(fd, Cf1_file, pkt->len) == pkt->len;
            break;

        case T_FILE_C0:
            return recv_file(fd,  C0_file, pkt->len) == pkt->len;
            break;

        case T_FILE_CU0:
            return recv_file(fd, CU0_file, pkt->len) == pkt->len;
            break;

        case T_FILE_T:
            return recv_file(fd, T_file, pkt->len) == pkt->len;
            break;

        case T_QUERY:
            printf("query.\n");
            break;

        default:
            return 0;
    }
    return 1;
}
void handle_client(int fd)
{
    char wel[] = "Welcome to use vdb server.\n";
    const int len = sizeof(struct packet);
    struct packet *pkt = (struct packet *)rcv_buf;
    write(fd, wel, sizeof(wel));
    while(1)
    {
        if(read_all(fd, (char*)pkt, len) != len)
        {
            printf("read pkt failed!\n");
            break;
        }
        if(!handle_pkt(fd, pkt))
        {
            printf("Handle pkt failed!\n");
            break;
        }
    }
    close(fd);
}
void reload()
{
    char *gv[3]={"main","param/a.param", NULL};
    vdb_init_read(&ss, &q,  2,  gv);
}
void run_server(int fd)
{
    if(be_reload)
        reload();
    int client_fd = -1;
    printf("Server is running.\n");
    while(1)
    {
        client_fd = accept(fd, NULL, 0);
        if(fd<0)
        {
            printf("accpet failed!\n");
            exit(-1);
        }
        printf("accept a client.fd=%d\n", client_fd);
        handle_client(client_fd);
    }
}
int main()
{

    run_server(start_listen());
    return 0;
}
