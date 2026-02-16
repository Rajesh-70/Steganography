#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    //step1 -> check_operation_type(argv[1])
    OperationType ret = check_operation_type(argv[1]);
    //step2 -> check the return value == e_encode
    if(ret == e_encode)
    {
        if(argc < 4)
        {
            printf("Error : invalid no of arguments\n");
            return 1;
        }

        //declare structure variable EncodeInfo encInfo
        EncodeInfo encInfo;
        //--> read_and_validate_encode_args(pass command line arg, &encInfo) == e_success or e_failure
        int status = read_and_validate_encode_args(argv,&encInfo);
        
        // e_success -> next step.
        if(status = e_failure)
        {
            //e_failure -> print error msg and stop the program
            printf("Error : invalid extensions\n");
            return 1;
        }

        // call do_encoding(&encInfo);
        if(do_encoding(&encInfo) == e_failure)
        {
            printf("Encoded failure\n");
            return 1;
        }

        printf("##Encoded successfully##\n");
    }
    else if(ret == e_decode)//step3 -> return value == e_decode
    {
        if(argc < 3)
        {
            printf("Error : invalid no of arguments\n");
            return 1;
        }
        //declare structure variable EncodeInfo encInfo
        DecodeInfo decInfo;
        //--> read_and_validate_encode_args(pass command line arg, &encInfo) == e_success or e_failure
        // e_success -> next step.
        if(read_and_validate_decode_args(argv,&decInfo) == e_failure)
        {
            //e_failure -> print error msg and stop the program
            printf("Error : invalid extensions\n");
            return 1;
        }
        // e_success -> next step.
        
        // call do_encoding(&encInfo);
        if(do_decoding(&decInfo) == e_failure)
        {
            printf("Decoded failure\n");
            return 1;
        }

        printf("##Decoded successfully##\n");
    }//step3 -> return value == e_unsupported
    else if(ret == e_unsupported)
    {
        // --> print invalid arg
        printf("Error : invalid argument\n");
        return 1;
    }
}

OperationType check_operation_type(char *symbol)
{
    //step1 -> check it is -e or -d
    // if it is -e return e_encode
    // else if it is -d rteun e_decode
    if(symbol == NULL)
    {
        printf("Invalid no of arguments\n");
        return 1;
    }
    else if(strcmp(symbol,"-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(symbol,"-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
