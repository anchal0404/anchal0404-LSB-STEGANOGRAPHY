#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/*Function to read and validate the arguments passed in the cla.*/
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    //check argv[2] is bmp or not
    if(!strcmp(strchr(argv[2],'.'),".bmp"))
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        printf("Please pass bmp file\n");
        return e_failure;
    }
    if(argv[3]!=NULL)
    {
        encInfo -> secret_fname = argv[3];
        strcpy(encInfo -> extn_secret_file,strchr(argv[3],'.'));
    }
    else
    {
        printf("Please pass secret file\n");
        return e_failure;
    }
    if(argv[4] != NULL)
    {
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        encInfo -> stego_image_fname = "steged_img.bmp";
    }
    return e_success;
}

/*Function to check capacity of the bmp files to encode the secret data.
* Input: EncodeInfo -Image capacity and secret file size
* Output: Status: success if true
* Description: BMP Image size should be able to handle 1:8
*Ratio encoding excluding the header 54 bytes */

Status check_capacity(EncodeInfo *encInfo)
{
    //Check the size compatibility 
    encInfo->image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    //moving file offset of secret file to endif
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    
    if(encInfo-> image_capacity > ((strlen(MAGIC_STRING) + 4 +strlen(encInfo->extn_secret_file)+ 4 + encInfo -> size_secret_file) * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

//Move offset of the secret file to the end.
long get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}


/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

/*Check the size of bmp file.*/
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

/*Function definition to open files for encoding.*/
Status open_files(EncodeInfo *encInfo)
{
    if(encInfo->stego_image_fname == NULL)
    {
        printf("Output File not mentioned.Creating %s as default\n",encInfo->stego_image_fname);
    }
    printf("Opening required files\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    printf("Opening %s\n",encInfo->src_image_fname);
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    printf("Opening %s\n",encInfo->secret_fname);
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    printf("Opening %s\n",encInfo->stego_image_fname);
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }
    // No failure return e_success
    return e_success;
}

/*Function Definition to copy bmp file header.
 * Inputs: Src Image file and Stego Image file pointers
 * Output: Status: Success if true
 * Description: e_success or e_failure, on 54 byte copy errors */

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char str[54];
    rewind(fptr_src_image);
    fread(str,sizeof(char),54,fptr_src_image);
    fwrite(str, sizeof(char) , 54 ,fptr_dest_image);
    return e_success;
}


/*Function defition to encode bytes to lsb of data
 * Inputs: 1 byte of data and 1 byte of image data
 * Output: Status: Success if true
 * Description: e_success or e_failure while getting errors
*/

Status encode_byte_to_lsb(char data,char *image_buffer)
{ 
    for(int i=0; i<8 ; i++)
    {
        image_buffer[i] = ((data & (1<<(7-i))) >> (7-i)) | (image_buffer[i] & 0xFE);
    }
}

/*Function defition to encode data to image*/ 
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char str[8];
	for(int i=0;i<size;i++)
	{
		//read 8 bytes of data from src image
		fread(str,sizeof(char),8,fptr_src_image);
		encode_byte_to_lsb(data[i],str);
		//write 8 bytes of data into stego image
		fwrite(str,sizeof(char),8,fptr_stego_image);
	}
}

/*Function definition to encode size of magic string*/
Status encode_size_to_lsb(int size, char *image_buffer)
{
    for(int i = 0;i < 32;i++)
    {
        image_buffer[i]=((size & (1<<(31-i))) >>(31-i)) | (image_buffer[i] & 0xFE);
    }
}


/*Function defintion to encode magic string.*/
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo ->fptr_stego_image);
    return e_success;
}

/*Function definition to encode size of extenstion of secret file*/
Status encode_secret_file_extn_size(int extn_size,EncodeInfo *encInfo)
{
    char str[32];
    fread(str,32,sizeof(char),encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size,str);
    fwrite(str,32,sizeof(char),encInfo->fptr_stego_image);
    return e_success;
}

/*Function defition to encode secret file extention*/
Status encode_secret_file_extn(const char *file_extn,EncodeInfo *encInfo)
{
    encode_data_to_image(encInfo->extn_secret_file,strlen(encInfo->extn_secret_file),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/*Function definition to encode secret file size*/
Status encode_secret_file_size(long file_size,EncodeInfo *encInfo)
{
    char str[32];
    fread(str,32,sizeof(char),encInfo->fptr_src_image);
    encode_size_to_lsb((int)file_size,str);
    fwrite(str,32,sizeof(char),encInfo->fptr_stego_image);
    return e_success;
}

/*Function definition to encode secret file data 
 * Inputs: Secret file data, image file and Stego Image file pointers
 * Output: Status: Success if true
 * Description: e_success or e_failure if errors
 */ 
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);
    fread(str,encInfo->size_secret_file,sizeof(char),encInfo->fptr_secret);
    encode_data_to_image(str,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/*Function definition encode remaining data in output file
 * Inputs: Image file and Stego Image file pointers
 * Output: Status: Success if true.
 * Description: e_success or e_failure while getting errors.
 */
Status copy_remaining_img_data(FILE *fptr_src,FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1 ,sizeof(char), fptr_src)> 0)
    {
        fwrite(&ch,1,sizeof(char),fptr_dest);
    }
    return e_success;
}

//Function to pass the statements about the processes information.
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("Done\n");
    }
    else
    {
        printf("\nFailed to open the files\n");
        return e_failure;
    }

    printf("## Encoding Procedure Started ##\n");
    if(check_capacity(encInfo) == e_success)
    {
        printf("Checking for beautiful.bmp capacity to handle %s\nDone. Found OK",encInfo->secret_fname);
    }
    else
    {
        printf("\nFailed to check the capacity\nNot valid");
        return e_failure;
    }
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo ->fptr_stego_image)== e_success)
    {
        printf("\nCopying Image Header\nDone");
    }
    else
    {
        printf("\nFailed to copy the Image Header");
        return e_failure;
    }

    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("\nEncoding Magic String Signature\nDone");
    }
    else
    {
        printf("\nFailed to Encode Magic String Signature\n");
        return e_failure;
    }

    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo) == e_success)
    {
        printf("\nEncoding secret.txt File Extension size\nDone");
    }
    else
    {
        printf("\nFailed to encode secret.txt file Extension size\n");
        return e_failure;
    }

    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)== e_success)
    {
        printf("\nEncoding sec File Extension\nDone");
    }
    else
        {
            printf("\nFailed to encode secret.txt File Extension\n");
            return e_failure;
        }

    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)
    {
        printf("\nEncoding secret.txt File Size\nDone");
    }
    else
    {
        printf("\nFailed to encode the secret.txt File Size\n");
        e_failure;
    }

    if(encode_secret_file_data(encInfo)==e_success)
    {
        printf("\nEncoding secret.txt File data \nDone");
    }
    else
    {
        printf("\nFailed to encode secret.txt Filedata.\n");
        return e_failure;
    }

    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
    {
        printf("\nCopying Left Over Data\nDone");
    }
    else
    {
        printf("\nFailed to copy remaining data.\n");
        return e_failure;
    }
    return e_success;
}
