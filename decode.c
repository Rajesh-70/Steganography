#include<stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //step1 -> check source file name having .bmp present or not
    int len = strlen(argv[2]);
    if(strcmp(&argv[2][len - 4],".bmp") != 0)
    {
        // no -> return e_failure
        return e_failure;
    }
    // yes -> store source file name into encInfo->stego_image_fname
    decInfo -> stego_image_fname = argv[2];

    //strp -> check, user give destination file or not
    if(argv[3] != NULL)
    {
        //copy destination file name without extension
        strcpy(decInfo->output_fname, argv[3]);

        strtok(decInfo -> output_fname,".");
        /*char *dot = strrchr(decInfo->output_fname,'.');
        if(dot) 
        {
            *dot = '\0';
        }*/
    }
    else
    {
        strcpy(decInfo->output_fname,"output");
    }
    //step -> 3 return e_success
     return e_success;
}

//open decode files
Status open_decode_files(DecodeInfo *decInfo)
{
    printf("Opening required files\n");
    // open stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }
    printf("Opened %s\n",decInfo -> stego_image_fname);
    // No failure return e_success
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    printf("Decoding Magic String Signature\n");
    //step1 -> skip bmp header
    fseek(decInfo -> fptr_stego_image,54,SEEK_SET);
    //step2 -> read 16 bytes from stego file
    char buff[8];
    char ch;
    for(int i = 0;i < strlen(magic_string);i++)
    {
        //read first 8 bytes from stego files 
        fread(buff,1,8,decInfo -> fptr_stego_image);
        //call decode byte to lsb
        if(decode_byte_to_lsb(&ch,buff) == e_failure)
        {
            return e_failure;
        }
        //check magic string matched or not
        if(ch != magic_string[i])
        {
            printf("Error : Magic string not matched.Decoding failed\n");
            return e_failure;
        }
    }
    // return e_success;
    return e_success;
}


//decode secret file extension size 
Status decode_secret_file_extn_size(int *size,DecodeInfo *decInfo)
{
    printf("Decoding Output File Extenstion size\n");
    //step1 -> read 32 bytes of buffer from stego image
    char buff[32];
    fread(buff,1,32,decInfo -> fptr_stego_image);

    // call encode_size_to_lsb(size, buffer)
    if(decode_size_to_lsb(size,buff) == e_failure)
    {
        return e_failure;
    }

    printf("Secret File Extension Size : %d\n",*size);

     //  return e_success;
    return e_success;
}


//decode secret file extension
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    printf("Decoding Output File Extenstion\n");
    //buffer to store 8 bytes
    char buff[8];
    char ch;
    char extn[20];
    for(int i = 0;i < decInfo -> secret_extn_size;i++)
    {
        //read 8 bytes frome src image and store in buffer
        fread(buff,1,8,decInfo -> fptr_stego_image);

        //call decode byte to lsb
        if(decode_byte_to_lsb(&ch,buff) == e_failure)
        {
            return e_failure;
        }
        //decode extn and store it in any array
        extn[i] = ch; 
    }
    extn[decInfo ->secret_extn_size] = '\0';

    printf("Secret file extensios is %s\n",extn);

    //concatinate both output file name and secret file extn
    strcat(decInfo -> output_fname,extn);

    //open output file 
    decInfo -> fptr_secret = fopen(decInfo -> output_fname,"wb");
    printf("Opened %s\n",decInfo -> output_fname);
    //Error validate
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n",decInfo -> output_fname);

        return e_failure;
    }

    //if no failure return e_succsess
    return e_success;
}

Status decode_secret_file_size(long *file_size, DecodeInfo *encInfo)
{
    printf("Decoding  File Size\n");
    char buff[32];
    int size;
    //step1 -> read 32 bytes of buffer from stego image
    fread(buff,1,32,encInfo -> fptr_stego_image);
    
    //step2 -> call size to lsb to get the secret file size
    if(decode_size_to_lsb(&size,buff) == e_failure)
    {
        return e_failure;
    }

    *file_size = size;
    printf("Secret file size : %ld\n",*file_size);

    //step3 -> return e_success
    return e_success;
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    printf("Decoding  File Data\n");
	char data;
	char buff[8];

    //read 8 bytes of buffer from stego image
	for(int i = 0; i < decInfo->size_secret_file; i++)
	{
        
		fread(buff,1,8,decInfo->fptr_stego_image);

		//encode secret byte into buffer
		if(decode_byte_to_lsb(&data,buff) == e_failure)
		{
			return e_failure;
		}

		//write decoded data to output file
		fwrite(&data,1,1,decInfo->fptr_secret);
	}

    //return e_success
	return e_success;
}

Status decode_byte_to_lsb(char *data, char *image_buffer)
{
    *data = 0;
    for(int i = 0;i < 8;i++)
    {  
       *data = *data << 1;
       *data = *data | (image_buffer[i] & 1); 
    }
    //printf("%c ",*data);
    return e_success;
}

Status decode_size_to_lsb(int *size, char *imageBuffer)
{
    *size = 0;
    for(int i = 0;i < 32;i++)
    {  
       *size = *size << 1;
       *size = *size | (imageBuffer[i] & 1); 
    }
    return e_success;
}


Status do_decoding(DecodeInfo *decInfo)
{
    //step1 -> call open file(decInfo)
    printf("## Decoding Procedure Started ##\n");
    if(open_decode_files(decInfo) == e_failure)
    {
        // e_failure -> print error msg and return e_failure
        return e_failure;
    }
    // e_success -> print success msg goto next step
    printf("Done.Opened required files\n");


	// step2 -> call decode magic string(MAGIC_STRING,decInfo)
    if(decode_magic_string(MAGIC_STRING,decInfo) == e_failure)
    {
        printf("Error : Unable to decode magic string\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done.\n");

	// step3 ->call decode_secret_file_extn_size(strlen(decInfo -> extn_secret_file),decInfo)
    if(decode_secret_file_extn_size(&decInfo -> secret_extn_size,decInfo) == e_failure)
    {
        printf("Error : unable to decode secret file extn size\n");
        return e_failure;
    }
	//print success msg to go to next step
    printf("Done\n");

	// step4 ->call decode_secret_file_extn(decInfo)
    if(decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("Error : unable to decode secret file extn\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step5 -> call decode_secret_file_size(decInfo -> size_secret_file,decInfo)
    if(decode_secret_file_size(&decInfo -> size_secret_file,decInfo) == e_failure)
    {
        printf("Error : unable to decode secret file size\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step6 -> call decode_secret_file_data(decInfo)
    if(decode_secret_file_data(decInfo) == e_failure)
    {
        printf("Error : unable to decode secret file data\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step7 -> retrun e_success
    return e_success;
}