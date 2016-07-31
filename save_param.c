#include <pbc/pbc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "save_param.h"
#define FILE_NAME_LEN
char g_file[FILE_NAME_LEN]="./param/g";
char hi_file[FILE_NAME_LEN]="./param/hi";
char hij_file[FILE_NAME_LEN]="./param/hij";
#define ELEMENT_MAX_LEN 2048
char buf[ELEMENT_MAX_LEN];
int save_g(element_t g)
{

    int n = element_length_in_bytes(g);
    int ret;
    if(n > ELEMENT_MAX_LEN)
    {
        pbc_die("element g too long.\n");
        return -1;
    }
    if(element_to_bytes(buf, g) != n)
    {
        pbc_die("element_to_btyes g failed!\n");
        return -1;
    }
    FILE *fp = fopen(g_file,"wb");
    if(NULL == fp)
    {
        pbc_die("error: open file %s failed!\n", g_file);
        return -1;
    }
    ret = fwrite(buf, n, 1, fp);
    if(ret != 1)
    {
        pbc_die("error:write g to file %s failed!\n", g_file);
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
        //pbc_die("error:write g to file %s failed!\n", g_file);
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

int save_hi(element_t *hi, int q, int max_len)
{
    int i;
    if(NULL == hi || q <=0 || max_len <=0)
        pbc_die("Bad param in save hi\n");

    if(max_len > ELEMENT_MAX_LEN)
    {
        pbc_die("max_len too long.\n");
        return -1;
    }
    FILE *fp = fopen(hi_file,"wb");
    if(NULL == fp)
    {
        pbc_die("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fwrite(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        pbc_die("save make_len failed.\n", i);
        return -1;
    }
    for(i = 0; i < q; i++)
    {
        int n = element_length_in_bytes_compressed(hi[i]);
        int ret;
        memset(buf, 0, max_len);
        if(n > ELEMENT_MAX_LEN || n > max_len)
        {
            fclose(fp);
            pbc_die("element hi[%d] too long.\n", i);
            return -1;
        }
        if(element_to_bytes_compressed(buf, hi[i]) != n)
        {
            fclose(fp);
            pbc_die("element_to_btyes hi[%d] failed!\n", i);
            return -1;
        }
        ret = fwrite(buf, n, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            pbc_die("error:write hi[%d] to file %s failed!\n", i, hi_file);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int read_hi(element_t *hi, int q)
{
    int i;
    int max_len;
    if(NULL == hi || q <=0)
        pbc_die("Bad param in save hi\n");

    FILE *fp = fopen(hi_file,"rb");
    if(NULL == fp)
    {
        pbc_die("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fread(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        pbc_die("save make_len failed.\n", i);
        return -1;
    }

    if(max_len > ELEMENT_MAX_LEN)
    {
        pbc_die("max_len too long.\n");
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
            pbc_die("error:read hi[%d] to file %s failed!\n", i, hi_file);
            return -1;
        }
        if(element_from_bytes_compressed(hi[i], buf) <=0)
        {
            fclose(fp);
            pbc_die("element_to_btyes hi[%d] failed!\n", i);
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
        pbc_die("Bad param in save hi\n");

    if(max_len > ELEMENT_MAX_LEN)
    {
        pbc_die("max_len too long.\n");
        return -1;
    }
    FILE *fp = fopen(hij_file,"wb");
    if(NULL == fp)
    {
        pbc_die("error: open file %s failed!\n", hij_file);
        return -1;
    }

    if(fwrite(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        pbc_die("save make_len failed.\n", i);
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
            pbc_die("element hij[%d] too long.\n", i);
            return -1;
        }
        if(element_to_bytes_compressed(buf, hij[i*q+j]) != n)
        {
            fclose(fp);
            pbc_die("element_to_btyes hij[%d][%d] failed!\n", i,j);
            return -1;
        }
        ret = fwrite(buf, n, 1, fp);
        if(ret != 1)
        {
            fclose(fp);
            pbc_die("error:write hij[%d][%d] to file %s failed!\n", i, j, hij_file);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int read_hij(element_t *hij, int q)
{
    int i;
    int j;
    int max_len = 0;
    if(NULL == hij || q <=0)
        pbc_die("Bad param in save hi\n");

    FILE *fp = fopen(hij_file,"rb");
    if(NULL == fp)
    {
        pbc_die("error: open file %s failed!\n", hi_file);
        return -1;
    }

    if(fread(&max_len, sizeof(max_len), 1, fp) != 1)
    {

        fclose(fp);
        pbc_die("read max_len failed.\n", i);
        return -1;
    }

    if(max_len > ELEMENT_MAX_LEN)
    {
        pbc_die("max_len too long.\n");
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
            pbc_die("error:write hij[%d][%d] to file %s failed!\n", i, j,  hij_file);
            return -1;
        }
        if(element_from_bytes_compressed(hij[i*q+j], buf) <=0)
        {
            fclose(fp);
            pbc_die("element_to_btyes hi[%d][%d] failed!\n", i,j);
            return -1;
        }

    }
    fclose(fp);
    return 0;
}

