/*Include header files*/
#include<stdio.h>
#include "types.h"
#include "decode.h"
#include<string.h>
#include"common.h"
#include<stdlib.h>

/*Function definition to read and validate command line arguments*/
Status_d read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    if(!strcmp(strchr(argv[2],'.'),".bmp"))
    {
        decInfo -> dsrc_image_fname = argv[2];
    }
    else
    {
        printf("Please pass bmp file\n");
        return d_failure;
    }

    if(argv[3] !=NULL)
    {
        decInfo -> dsecret_fname = argv[3];
    }
    else
    {
        decInfo -> dsecret_fname = "decoded.txt";
    }
    return d_success;
}

/*Function definition to open .bmp file for decoding*/
Status_d open_files_dec(DecodeInfo *decInfo)
{
    printf("Opening required files\n");
    decInfo -> fptr_d_src_image = fopen(decInfo -> dsrc_image_fname,"r");
    //printf("Opened %s\n",decInfo->dsrc_image_fname);
    //Do error handling
    if(decInfo -> fptr_d_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,"Error: Unable to to open file %s\n",decInfo->dsrc_image_fname);
        return d_failure;
    }
    return d_success;
}

/*Function definition to decode magic string from .bmp file*/
Status_d decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_d_src_image,54,SEEK_SET);
    int k = strlen(MAGIC_STRING);

    decInfo-> magic_data  = malloc(k + 1);
    decode_data_from_image(k,decInfo->fptr_d_src_image,decInfo);

    decInfo-> magic_data[k] = '\0';
    if(!strcmp(decInfo->magic_data,MAGIC_STRING))
    {
        return d_success;
    }
    else
    {
        return d_failure;
    }
}


/*Function definition to create decoded.txt file if not given by user*/
Status_d create_decode_secret_file(DecodeInfo *decInfo)
{

    decInfo -> fptr_d_secret = fopen(decInfo -> dsecret_fname,"w");
    if(decInfo -> fptr_d_secret ==NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n", decInfo -> dsecret_fname);
        return d_failure;
    }
    if(decInfo->dsecret_fname ==NULL)
        printf("Output file not mentioned.Creating %s as default\n",decInfo->dsecret_fname);
    return d_success;
}

/*Function definition to decode data from image*/
Status_d decode_data_from_image(int size,FILE *fptr_d_src_image,DecodeInfo *decInfo)
{
    char str[8];
    for(int i=0;i< size;i++)
    {
        fread(str,8,sizeof(char),fptr_d_src_image);
        decode_byte_from_lsb(&decInfo-> magic_data[i],str);

    }
    return d_success;
}

/*Function definition to decode bytes from lsb*/
Status_d decode_byte_from_lsb(char * data,char *image_buffer)
{
    int bit = 7;
    unsigned char ch = 0x00;
    for(int i = 0; i < 8 ; i++)
    {
        ch = ((image_buffer[i] & 0x01) << bit --) | ch;
    }
    *data = ch;
    return d_success;
}

/*Function definition to decode size from lsb*/
Status_d decode_size_from_lsb(char *buffer,int *size)
{

    int i=31;
    int no = 0x00;
    for(int j = 0 ; j < 32 ; j++)
    {
        no= ((buffer[j] & 0x01) << i--) | no;
    }
    *size = no;
}

/*Function definition to decode file extension size*/
Status_d decode_file_extn_size(int size,FILE *fptr_d_src_image)
{
    char str[32];
    int len;

    fread(str,32,sizeof(char),fptr_d_src_image);
    decode_size_from_lsb(str,&len);
    if(len == size)
    {
        return d_success;
    }
    else
    {
        return d_failure;
    }

}

/*Function definition to decode extension size from image*/
Status_d decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    for(int i = 0; i< size;i++)
    {
        fread(decInfo -> dsrc_image_fname,8,1,fptr_d_src_image);
        decode_byte_from_lsb(&decInfo -> dextn_secret_file[i],decInfo-> dsrc_image_fname);

    }
    return d_success;
}

/*Function definiton to decode secret file extension from .bmp*/
Status_d decode_secret_file_extn(char *file_ext,DecodeInfo *decInfo)
{
    file_ext = ".txt";
    int k = strlen(file_ext);
    decInfo->dextn_secret_file = malloc(k+1);
    decode_extension_data_from_image(k,decInfo->fptr_d_src_image,decInfo);

    decInfo-> dextn_secret_file[k] = '\0';

    if(!strcmp(decInfo-> dextn_secret_file,file_ext))
    {
        return d_success;
    }
    else
    {
        return d_failure;
    }
}

/*Function definition to decode secret file size */
Status_d decode_secret_file_size(int file_size,DecodeInfo *decInfo)
{
    char str[32];
    fread(str,32,sizeof(char),decInfo-> fptr_d_src_image);
    decode_size_from_lsb(str,&file_size);
    decInfo -> size_secret_file=file_size;
    //printf("%u\n",file_size);
    return d_success;
}

/*Function definition to decode secret file data */
Status_d decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    for(int i = 0; i< decInfo-> size_secret_file;i++)
    {
        fread(decInfo -> dsrc_image_fname,8,sizeof(char),decInfo-> fptr_d_src_image);
        decode_byte_from_lsb(&ch,decInfo-> dsrc_image_fname);
        fputc(ch,decInfo-> fptr_d_secret);
    }
    return d_success;
}

/*Function definition for reflecting information about decoding processes*/
Status_d do_decoding(DecodeInfo *decInfo)
{
    if(open_files_dec(decInfo) == d_success)
    {
        printf("Opened %s\n",decInfo->dsrc_image_fname);
    }
    else
    {
        printf("Failed to open the files\n");
        return d_failure;
    }

    if(decode_magic_string(decInfo) == d_success)
    {
        printf("Decoding Magic String Signature\nDone\n");
    }
    else
    {
        printf("Decoding Magic String Signature failed\n");
        return d_failure;
    }

    if(decode_file_extn_size(strlen(".txt"),decInfo -> fptr_d_src_image) == d_success)
    {
        printf("Decoding Output File Extension Size\nDone\n");
    }
    else
    {
        printf("Decoding Output File Extension Size Failed\n");
        return d_failure;
    }

    if(create_decode_secret_file(decInfo) == d_success)
    {
        printf("Opened %s\nDone.Opened all required files\n",decInfo->dsecret_fname);
    }
    else
    {
        printf("Creating %s file failed\n",decInfo->dsecret_fname);
        return d_failure;
    }

    if(decode_secret_file_extn(decInfo-> dextn_secret_file,decInfo) == d_success)
    {
        printf("Decoding Output File Extension\nDone\n");
    }
    else
    {
        printf("Decoding Output File Extension failed\n");
        return d_failure;
    }

    if(decode_secret_file_size(decInfo->size_secret_file,decInfo)== d_success)
    {
        printf("Decoding %s File size\nDone\n",decInfo->dsecret_fname);
    }
    else
    {
       printf("Decoding %s File size failed\n",decInfo-> dsecret_fname);
       return d_failure;
    }

    if(decode_secret_file_data(decInfo) == d_success)
    {
        printf("Decoding %s File Data\nDone\n",decInfo->dsecret_fname);
    }
    else
    {
        printf("Decoding %s File data failed\n",decInfo->dsecret_fname);
        return d_failure;
    }

    return d_success;
}


