#include <pbc/pbc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "save_param.h"
#define FILE_NAME_LEN
char g_file[FILE_NAME_LEN]="./param/g";
char q_file[FILE_NAME_LEN]="./param/q";
char hi_file[FILE_NAME_LEN]="./param/hi";
char hij_file[FILE_NAME_LEN]="./param/hij";
#define ELEMENT_MAX_LEN 2048
char buf[ELEMENT_MAX_LEN];

int save_ele(element_t g, const char *fname)
{

    int n = element_length_in_bytes(g);
    int ret;
    if(n > ELEMENT_MAX_LEN)
    {
        printf("element %s too long. \n", fname);
        return -1;
    }
    if(element_to_bytes(buf, g) != n)
    {
        printf("element_to_btyes %s failed!\n", fname);
        return -1;
    }
    FILE *fp = fopen(fname,"wb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", fname);
        return -1;
    }
    ret = fwrite(buf, n, 1, fp);
    if(ret != 1)
    {
        printf("error:write %s to file  failed!\n", fname);
        return -1;
    }
    fclose(fp);
    return 0;
}
int read_ele(element_t g, const char *fname)
{
    FILE *fp = fopen(fname,"rb");
    int ret;
    if(NULL == fp)
    {
        return -1;
    }
    ret = fread(buf, 1, ELEMENT_MAX_LEN, fp);
    if(ret <=0 )
    {
        //printf("error:write g to file %s failed!\n", g_file);
        return -1;
    }
    fclose(fp);
    ret = element_from_bytes(g, buf);
    if(ret <= 0)
    {
        return -1;
    }
    return 0;
}


int save_g(element_t g)
{

    int n = element_length_in_bytes(g);
    int ret;
    if(n > ELEMENT_MAX_LEN)
    {
        printf("element g too long.\n");
        return -1;
    }
    if(element_to_bytes(buf, g) != n)
    {
        printf("element_to_btyes g failed!\n");
        return -1;
    }
    FILE *fp = fopen(g_file,"wb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", g_file);
        return -1;
    }
    ret = fwrite(buf, n, 1, fp);
    if(ret != 1)
    {
        printf("error:write g to file %s failed!\n", g_file);
        return -1;
    }
    fclose(fp);
    return 0;
}

int read_g(element_t g)
{
    FILE *fp = fopen(g_file,"rb");
    int ret;
    if(NULL == fp)
    {
        return -1;
    }
    ret = fread(buf, 1, ELEMENT_MAX_LEN, fp);
    if(ret <=0 )
    {
        //printf("error:write g to file %s failed!\n", g_file);
        return -1;
    }
    fclose(fp);
    ret = element_from_bytes(g, buf);
    if(ret <= 0)
    {
        return -1;
    }
    return 0;
}

int save_arr(element_t *hi, int q, int max_len, const char *fname)
{
    int i;
    if(NULL == hi || q <=0 || max_len <=0)
    {
        printf("Bad param in save %s\n", fname);
        return -1;
    }

    if(max_len > ELEMENT_MAX_LEN)
    {
        printf("max_len too long. max_len=%d\n", max_len);
        return -1;
    }
    FILE *fp = fopen(fname, "wb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fwrite(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        printf("save make_len failed.\n", i);
        return -1;
    }
    for(i = 0; i < q; i++)
    {
        int n = element_length_in_bytes(hi[i]);
        int ret;
        memset(buf, 0, max_len);
        if(n > ELEMENT_MAX_LEN || n > max_len)
        {
            fclose(fp);
            printf("element [%d] too long. n=%d\n", i, n);
            return -1;
        }
        if(element_to_bytes(buf, hi[i]) != n)
        {
            fclose(fp);
            printf("element_to_btyes [%d] failed!\n", i);
            return -1;
        }
        ret = fwrite(buf, n, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            printf("error:write[%d] to file failed!\n", i);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int read_arr(element_t *hi, int q, const char *fname)
{
    int i;
    int max_len = 0;
    if(NULL == hi || q <=0)
    {
        printf("Bad param in save hi\n");
        return -1;
    }

    FILE *fp = fopen(fname, "rb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fread(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        printf("save make_len failed.\n", i);
        return -1;
    }

    if(max_len > ELEMENT_MAX_LEN)
    {
        printf("max_len too long.\n");
        return -1;
    }

    for(i = 0; i < q; i++)
    {
        int ret;
        memset(buf, 0, max_len);
        ret = fread(buf, max_len, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            printf("error:read hi[%d] to file %s failed!\n", i, hi_file);
            return -1;
        }
        if(element_from_bytes(hi[i], buf) <=0)
        {
            fclose(fp);
            printf("element_to_btyes hi[%d] failed!\n", i);
            return -1;
        }

    }
    fclose(fp);
    return 0;
}


int save_hij(element_t *hij, int q, int max_len)
{
    int i;
    int j;
    if(NULL == hij || q <=0 || max_len <=0)
        printf("Bad param in save hij\n");

    if(max_len > ELEMENT_MAX_LEN)
    {
        printf("max_len too long.\n");
        return -1;
    }
    FILE *fp = fopen(hij_file,"wb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", hij_file);
        return -1;
    }

    if(fwrite(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        printf("save make_len failed.\n", i);
        return -1;
    }
    for(i = 0; i < q; i++)
        for(j = 0; j < q; j++)
    {
        int n = element_length_in_bytes_compressed(hij[i*q+j]);
        int ret;
        memset(buf, 0, max_len);
        if(n > ELEMENT_MAX_LEN || n > max_len)
        {
            fclose(fp);
            printf("element hij[%d] too long.\n", i);
            return -1;
        }
        if(element_to_bytes_compressed(buf, hij[i*q+j]) != n)
        {
            fclose(fp);
            printf("element_to_btyes hij[%d][%d] failed!\n", i,j);
            return -1;
        }
        ret = fwrite(buf, n, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            printf("error:write hij[%d][%d] to file %s failed!\n", i, j, hij_file);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int read_hij(element_t *hij, int q, const char *fname)
{
    int i;
    int j;
    int max_len = 0;
    if(NULL == hij || q <=0)
        printf("Bad param in save hi\n");

    FILE *fp = fopen(fname,"rb");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fread(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        printf("read max_len failed.\n", i);
        return -1;
    }

    if(max_len > ELEMENT_MAX_LEN)
    {
        printf("max_len too long.\n");
        return -1;
    }
    printf("max_len=%d\n", max_len);
    for(i = 0; i < q; i++)
    for(j = 0; j < q; j++)
    {
        int ret;
        //if(i == j) continue;
        memset(buf, 0, max_len);
        ret = fread(buf, max_len, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            printf("error:write hij[%d][%d] to file %s failed!\n", i, j,  hij_file);
            return -1;
        }
        if(element_from_bytes_compressed(hij[i*q+j], buf) <=0)
        {
            fclose(fp);
            printf("element_to_btyes hi[%d][%d] failed!\n", i,j);
            return -1;
        }

    }
    fclose(fp);
    return 0;
}

int save_int(int q, const char *fname)
{
    FILE *fp = fopen(fname,"w");
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", fname);
        return -1;
    }
    int ret = fprintf(fp, "%d\n", q);//fwrite(&q, sizeof(q), 1, fp);
    if(ret <0)
    {
        printf("error:write  to file %s failed!\n", fname);
        return -1;
    }
    fclose(fp);
    return 0;

}

int read_int(int *q, const char *fname)
{
    FILE *fp = fopen(fname,"r");
    *q= - 1;
    if(NULL == fp)
    {
        printf("error: open file %s failed!\n", fname);
        return -1;
    }
    int ret = fscanf(fp, "%d", q);//fread(q, sizeof(*q), 1, fp);
    if(*q==-1)
    {
        printf("error:read  to file %s failed!\n", fname);
        return -1;
    }
    fclose(fp);
    return 0;

}
