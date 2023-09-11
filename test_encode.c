#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    //Function call to check operation to be performed
    int ret=check_operation_type(argv);
    if(ret == e_encode)
    {
        printf("## Encoding Started ##\n");
        //Declaring cariable for structure
        EncodeInfo encInfo;
        //Function call to read_and_validate_encode_args
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)
        {
            printf("Read and validate enc args successful\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("\n## Encoding Done Successfully ##\n");
            }
            else
            {
                printf("## Encoding is failed ##\n");
            }
        }
        else 
        {
            printf("Read and validate enc args failed\n");
        }
    }
    //Decoding operation
    else if(ret == e_decode)
    {
        printf("## Decoding Procedure Started ##\n");

        //Declare structure variables
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv,&decInfo) == d_success)
        {
            printf("Read and valid decoding arguments successful\n");
            if(do_decoding(&decInfo) == d_success)
            {
                printf("## Decoding Done Successfully ##\n");
            }
            else
            {
                printf("## Decoding is failed ##\n");

            }
        }
        else
        {
            printf("Read and validate decoding arguments failed\n");

        }
    }
    else
    {
        printf("Invalid option\nKindly pass for\nEncoding: ./a.out -e beautiful.bmp secret.txt stego.bmp\nDecoding: ./a.out -d stego.bmp decode.txt\n");
    }
    return 0;
}

//Function definition to check operation to be performed. 
OperationType check_operation_type(char *argv[])
{
    if(!strcmp(argv[1],"-e"))
        return e_encode;
    else if(!strcmp(argv[1],"-d"))
        return e_decode;
    else
        return e_unsupported;
}

