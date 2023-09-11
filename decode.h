#ifndef DECODE_H
#define DECODE_H

#include "types.h" //Contains user defined datatypes.

/*
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUFF_SIZE 1
#define MAX_IMAGE_BUFF_SIZE (MAX_SECRET_BUFF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
typedef struct _DecodeInfo
{
    /*Stego image Info*/

    char *dsrc_image_fname;
    FILE *fptr_d_src_image;

    char dimage_data[MAX_IMAGE_BUFF_SIZE];
    char *magic_data;
    char *dextn_secret_file;
    
    int size_secret_file;
    FILE *fptr_d_dest_image;

    char *dsecret_fname;
    FILE *fptr_d_secret;
}DecodeInfo;

/*Decoding Function Prototypes*/

/*Read and validate decode args from argv*/
Status_d read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);

/*Perform the decoding*/
Status_d do_decoding(DecodeInfo *decInfo);

/*Get the file pointer for i/p and o/p files*/
Status_d open_files_dec(DecodeInfo *decInfo);

/*Decode Magic Strings*/
Status_d decode_magic_string(DecodeInfo *decInfo);

/*Creating secret file for decoding*/
Status_d create_decode_secret_file(DecodeInfo *decInfo);

/*Decode data from Image*/
Status_d decode_data_from_image(int size,FILE *fptr_d_src_image,DecodeInfo *decInfo);

/*Decode byte from lsb*/
Status_d decode_byte_from_lsb(char* data,char *image_buffer);

/*Decode file extension size*/
Status_d decode_file_extn_size(int size,FILE *fptr_d_src_image);

/*Decode size from lsb*/
Status_d decode_size_from_lsb(char *buffer,int *size);

/*Decode secret file extension*/
Status_d decode_secret_file_extn(char *file_ext,DecodeInfo *decInfo);

/*Decode extension data from image */
Status_d decode_extension_data_from_image(int size,FILE *fptr_d_src_image,DecodeInfo *decInfo);

/*Decode secret file size*/
Status_d decode_secret_file_size(int file_size,DecodeInfo *decInfo);

/*Decode secret file data*/
Status_d decode_secret_file_data(DecodeInfo *decInfo);

#endif
