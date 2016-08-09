#include <stdio.h>
#include <stdlib.h>
#include <pbc/pbc.h>

int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        printf("Usage: ./param_gen rbit qbit outfile\n");
        exit(-1);
    }
    pbc_param_t pp;
    pbc_param_init_a_gen(pp, atoi(argv[1]), atoi(argv[2]));
    FILE *fp = fopen(argv[3], "w");
    if(fp == NULL)
    {
        printf("can't create file %s.\n", argv[3]);
    }
    pbc_param_out_str(fp, pp);
    fclose(fp);
    pbc_param_clear(pp);
    return 0;
}
