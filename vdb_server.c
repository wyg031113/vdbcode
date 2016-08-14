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

/*以下参数都是VDB需要的，把
 *他们都保存到当前目录入sparam
 *目录中。
 */
#define FILE_NAME_LEN 128
#define DEFAULT_LISTEN_PORT 7788
void restart();
static unsigned short listen_port = DEFAULT_LISTEN_PORT;  //服务器监听端口
static char q_file[FILE_NAME_LEN]="./sparam/q";
static char hij_file[FILE_NAME_LEN]="./sparam/hij";
static char C0_file[FILE_NAME_LEN] = "./sparam/C0";
static char CU0_file[FILE_NAME_LEN] = "./sparam/CU0";
static char Cf1_file[FILE_NAME_LEN] = "./sparam/Cf1";
static char H0_file[FILE_NAME_LEN] = "./sparam/H0";
static char T_file[FILE_NAME_LEN] = "./sparam/T";
static char aparam_file[FILE_NAME_LEN] = "./sparam/a.param";
static char Prog_file[FILE_NAME_LEN] = "./sparam/Prog";
static char listen_port_file[FILE_NAME_LEN] = "./sparam/listen_port";
static char mysql_conf_file[FILE_NAME_LEN] = "./sparam/mysql_conf";
struct setup_struct ss;
struct aux_struct as;
struct proof_tao tao;
static int q = 0; //数据库大小

/**从文件中读入需要的参数
 */
int vdb_init_read(struct setup_struct *ss,int *tq,  int argc, char *argv[])
{
    mpz_t v;
	int i, j, q = 0;
	struct pp_struct *pp;
    if(read_int(&q, q_file) != 0)
        return -1;
    *tq = q;

	memset(ss, 0, sizeof(struct setup_struct));
	ss->T = 0;

	pp = malloc(sizeof(struct pp_struct));
	if(NULL == pp)
		goto out4;
    pp->q = q;

    if(0 != init_Hij(pp))
		goto out2;

	pbc_demo_pairing_init(pp->pairing, argc, argv); //init G1 G2

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

	//element_pp_init(gpp, pp->g);
	pbc_info("Begin load hij\n");
	for(i = 0; i < q; i++)
		for(j = i; j < q; j++)
		{
			element_init_G1(pp->hij[i*q+j], pp->pairing);
			element_init_G1(pp->hij[j*q+i], pp->pairing);

		}

    if(read_hij(pp->hij, q, hij_file) !=0)
    {
        pbc_error("read hij failed!\n");
        return -1;
    }
     pbc_info("read hij from file!\n");

	//element_pp_clear(gpp);


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
        element_init_G1(ss->H0, pp->pairing);

    //read all
    read_int(&(ss->T), T_file);
    read_ele(ss->H0, H0_file);
    read_ele(ss->PK.C0, C0_file);
    read_ele(ss->PK.Cf1, Cf1_file);
    read_ele(ss->PK.CU0, CU0_file);
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

/**开始监听端口
 */
int listen_fd = -1;
int start_listen(void)
{
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(listen_port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    if((listen_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket failed!\n");
        exit(-1);
    }

    int optval = 1;
    socklen_t len = sizeof(optval);
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, len);

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
    printf("listen on port:%d\n", listen_port);
    return listen_fd;

}

/*初始化进度文件
 */
int write_prog_file(int prog)
{
    FILE *fp = fopen(Prog_file, "w");
    if(fp == NULL)
    {
        printf("open prog file failed!\n");
        return -1;
    }
    int ret = fprintf(fp, "%d\n", prog);

    fclose(fp);
    if(ret <= 0)
        return -1;
    else
        return 0;

}

/*获取初始化进度
 */
int get_prog(void)
{
    FILE *fp = fopen(Prog_file, "r");
    int prog = 0;
    if(fp == NULL)
    {
        if(write_prog_file(P_UNINIT) ==-1)
            return -1;
        return P_UNINIT;
    }
    int ret = fscanf(fp, "%d\n", &prog);

    fclose(fp);
    if(ret <= 0)
        return -1;
    else
        return prog;
}
#define BUF_SIZE 2048
static char rcv_buf[BUF_SIZE];

int be_reload = 1;
int be_inited = 0;

/*处理查询，返回证据
 */
int handle_query(int fd, struct packet *pkt)
{
    int x = 0; //查询数据库第x行
    if(pkt->len != sizeof(x))
    {
        printf("x len error!\n");
        return 0;
    }
    if(read_all(fd,(char*)&x, sizeof(x)) != sizeof(x))
    {
        printf("receive x failed!\n");
        return 0;
    }

    /*//从客户端接收hij 这里只接收头部，数据在计算证据pai时接收
    struct packet header_hij;
    if(read_all(fd,(char*)&header_hij, sizeof(header_hij)) != sizeof(header_hij))
    {
        printf("receive header hij failed!\n");
        return 0;
    }
*/
    if(x >=q || x <0)
    {
        printf("x is out of range!\n");
        return 0;
    }

    //计算关键证据pai
    element_t pai;
    element_init_G1(pai, ss.S.pp->pairing);
    if(vdb_query_paix(pai, &ss, x, fd) !=0)
    {
        printf("query paix failed!\n");
        return 0;
    }

    //发送证据到客户端
    struct packet rep;
    rep.type = T_REPLY_QUERY;
    rep.len = element_length_in_bytes(pai);
    static char buf[1024]; //注意，静态buf
    if(rep.len > 1024)
    {
        printf("element pai too long!\n");
        element_clear(pai);
        return 0;
    }
    printf("x=%d pai len=%d\n", x, rep.len);
    element_to_bytes(buf, pai);
    //memset(&rep,0xff,sizeof(rep));
    if(write_all(fd, (char*)&rep, sizeof(rep)) == sizeof(rep) &&
       write_all(fd, buf, rep.len) == rep.len)
    {
        printf("replay query finished!\n");
        element_clear(pai);
        return 1;
    }
    else
    {
        printf("Send replay data failed!\n");
        element_clear(pai);
        return 0;
    }
}
/**
 * pkt len is BUF_SIZE
 * 处理请求
 */
int handle_pkt(int fd, struct packet *pkt)
{
    int ret = 0;
    switch(pkt->type)
    {
        case T_FILE_Q:
            printf("Receive file q.\n");
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, q_file, pkt->len) == pkt->len;
            break;
        case T_FILE_HIJ:
            printf("Receive file hij.\n");
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, hij_file, pkt->len) == pkt->len;
            break;
        case T_FILE_H0:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, H0_file, pkt->len)== pkt->len;
            break;

        case T_FILE_Cf1:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, Cf1_file, pkt->len) == pkt->len;
            break;

        case T_FILE_C0:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd,  C0_file, pkt->len) == pkt->len;
            break;

        case T_FILE_CU0:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, CU0_file, pkt->len) == pkt->len;
            break;

        case T_FILE_T:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, T_file, pkt->len) == pkt->len;
            break;
         case T_MYSQL_CONF:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, mysql_conf_file, pkt->len) == pkt->len;
            break;


        case T_FILE_APARAM:
            write_prog_file(P_INITING);
            be_reload = 1;
            return recv_file(fd, aparam_file, pkt->len) == pkt->len;
            break;
        case T_FINISH:  //接收参数结束
            write_prog_file(P_FINISH);
            be_reload = 1;//重新加载参数
            printf("Init file receive finished.\n");
            return 1;
            break;
        case T_QUERY:
            printf("query.\n");
            if(get_prog() == P_FINISH)
                return handle_query(fd, pkt);;
            break;

        default:
            return 0;
    }
    return 1;
}
static void reload();
void handle_client(int fd)
{
    char wel[] = "Welcome to use vdb server.\n";
    const int len = sizeof(struct packet);
    struct packet *pkt = (struct packet *)rcv_buf;
    //write(fd, wel, sizeof(wel));
    while(1)
    {
        if(be_reload && get_prog() == P_FINISH)
            restart();
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

/*重新加载参数
 *
 */

void restart()
{
    printf("restarting server, please wait...!\n");
    close(listen_fd);
    destroy_db();
    char *argv[]={"./vdb_server",NULL};
    execv(argv[0], argv);
    printf("restart server failed!\n");
    exit(-1);
}

/**加载参数
 */
static void reload()
{
    char *gv[3]={"main","param/a.param", NULL};
    if(get_prog() == P_FINISH)
    {
        printf("reload params, please wait...\n");
        if(vdb_init_read(&ss, &q,  2,  gv) != 0)
            write_prog_file(P_UNINIT);
        else
        {
            printf("Re init database.\n");
            destroy_db();
            init_db(q, mysql_conf_file);
        }
    }
    if(read_int(&listen_port, listen_port_file) !=0)
    {
        save_int(DEFAULT_LISTEN_PORT, listen_port_file);
        listen_port = DEFAULT_LISTEN_PORT;
    }
    be_reload = 0;
}

/**服务器循环
 */
void run_server(int fd)
{
    int client_fd = -1;
    printf("Server is running.\n");
    while(1)
    {
        if(be_reload && get_prog() == P_FINISH)
            restart();
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
int main(int argc, char *argv[])
{
    change_dir(argv[0]);  //更改运行目录，以方便寻找参数文件
    be_inited = get_prog();
    reload();
    init_db(q, mysql_conf_file);
    run_server(start_listen());
    return 0;
}
