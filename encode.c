#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
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

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;

}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //step1 -> check source file name having .bmp present or not
    int len = strlen(argv[2]);
    if(strcmp(&argv[2][len - 4],".bmp") != 0)
    {
        // no -> return e_failure
        return e_failure;
    }
    // yes -> store source file name into encInfo->src_image_fname
    encInfo -> src_image_fname = argv[2];


    //step2 -> check secret file having extn or not
    encInfo->secret_fname = argv[3];
    char *dot = strrchr(argv[3], '.');
    if(dot == NULL)
    {
        printf("No extension\n");
        return e_failure;
    }
    len = strlen(dot + 1);   // skip '.'
    if(len < 1 || len > 3)
    {
        printf("Extension must be 1 to 3 characters\n");
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file, dot);


    // step3 -> check optional file is passed or not
    if(argv[4] != NULL)
    {
        // yes -> check the file having .bmp or not
        len = strlen(argv[4]); 
        if(strcmp(&argv[4][len - 4],".bmp") != 0)
        {
            // no -> return e_failure
            return e_failure;
        }
        // yes -> store the file name into encInfo->stego_image_fname
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        // no -> store default name to encInfo->stego_image_fname = "stego.bmp";
        encInfo -> stego_image_fname = "stego.bmp";
    }
    //step4 -> return e_success
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    printf("Opening required files\n");
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    printf("Opened Source file %s\n",encInfo -> src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }
    printf("Opened %s\n",encInfo -> secret_fname);


    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    printf("Opened %s\n",encInfo -> stego_image_fname);
    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // step1 -> encInfo->image_capacity =get_image_size_for_bmp(source_file_pointer)
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    printf("Capacity of source file = %d\n",encInfo -> image_capacity);
    // step2 -> find secret file size encInfo -> size_secret_file = get_file_size(secret file pointer)
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    printf("Capacity of secret file = %ld\n",encInfo -> size_secret_file);
    /* step3 -> compare encInfo->image_capacity > (54 bytes for header) + (8 * sizeof(magic string)) + 32(for secret file extension legnth(ex : .txt = legth 4))
     + 32(for extension characters (.,t,x,t)) + 32(for size of secret file) + (secret file data * 8)*/
    if(encInfo -> image_capacity >= (54 + (strlen(MAGIC_STRING)*8) + 32 + (strlen(encInfo ->extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)))
    {
        //yes -> return e_success
        printf("Checking for %s capacity to handle %s\n",encInfo -> src_image_fname,encInfo -> secret_fname);
        return e_success;
    }
    else
    {
        // no -> return e_failure
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //step1 -> rewind file pointer 0th pos
    fseek(fptr_src_image,0,SEEK_SET);

    char buff[54];
    //step2 -> read 54 bytes from source file(use fread);
    fread(buff,1,54,fptr_src_image);	
    printf("Copying Image Header\n");
    // step3 -> write 54 bytes to stego iamge file(from buffer)
    fwrite(buff,1,54,fptr_dest_image);

    if(ftell(fptr_dest_image)!=54)
    {
        return e_failure;
    }
    // return e_succes
    return e_success;
}


Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // step1 -> read 8 bytes of buffer from source image
    char buff[8];
    printf("Encoding Magic String Signature\n");
    for(int i = 0;i < strlen(magic_string);i++)
    {
        fread(buff,1,8,encInfo -> fptr_src_image);
        // step2 -> call encode_bytes_to_lsb(magic_string[0],buffer)
        if(encode_byte_to_lsb(magic_string[i],buff) == e_failure)
        {
            return e_failure;
        }
        // step3 -> store the buffer to stego image file
        fwrite(buff,1,8,encInfo -> fptr_stego_image);
               // do this upto magic string size time (step1)
    }
    // return e_success;
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    //step1 -> read 32 bytes of buffer from source image
    // call encode_size_to_lsb(size, buffer)
    // store the buffer to stego image file
    //  return e_success;
    char buff[32];

    fread(buff,1,32,encInfo -> fptr_src_image);
    printf("Encoding %s File Extenstion Size\n",encInfo -> secret_fname);
    if(encode_size_to_lsb(size,buff) == e_failure)
    {
        return e_failure;
    }
    fwrite(buff,1,32,encInfo -> fptr_stego_image);

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //buffer to store 8 bytes
    char buff[8];
    printf("Encoding %s File Extension\n",encInfo -> secret_fname);
    for(int i = 0;i < strlen(file_extn);i++)
    {
        //read 8 bytes frome src image and store in buffer
        fread(buff,1,8,encInfo -> fptr_src_image);

        //call encode byte to lsb
        if(encode_byte_to_lsb(file_extn[i],buff) == e_failure)
        {
            return e_failure;
        } 
        //write that 8 bytes to output/stego image files
        fwrite(buff,1,8,encInfo -> fptr_stego_image);
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    //step1 ->  read 32 bytes of buffer from source image
    char buff[32];
    fread(buff,1,32,encInfo -> fptr_src_image);
    printf("Encoding %s File Size\n",encInfo -> secret_fname);
    //step2 -> call encode_bytes_to_lsb(file_size,buffer)
    if(encode_size_to_lsb(file_size,buff) == e_failure)
    {
        return e_failure;
    }
    //step3 -> store the buffer to stego image file
    fwrite(buff,1,32,encInfo -> fptr_stego_image);

    //return e_success
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
	//step1 -> read secret data into one data buffer 
	//step2 -> read 8 bytes of buffer from source image
        //step3 -> call encode_byte_to_lsb(data,buffer)
        //step4 -> store the buffer to stego image file
              //repeat the process upto size of secret file(run a loop)
        //return e_success
	
	char data;
	char buff[8];

	//reset secret file pointer to beginning
	//fseek(encInfo->fptr_secret, 0, SEEK_SET);

    printf("Encoding %s File Data\n",encInfo -> secret_fname);
	for(int i = 0; i < encInfo->size_secret_file; i++)
	{
		//read 1 byte from secret file
		fread(&data,1,1,encInfo->fptr_secret);

		//read 8 bytes from source image
		fread(buff,1,8,encInfo->fptr_src_image);

		//encode secret byte into buffer
		if(encode_byte_to_lsb(data,buff) == e_failure)
		{
			return e_failure;
		}

		//write encoded buffer to stego image
		fwrite(buff,1,8,encInfo->fptr_stego_image);
	}

	return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    printf("Copying Left Over Data\n");
    while(fread(&ch,sizeof(char),1,fptr_src))
    {
        fwrite(&ch,sizeof(char),1,fptr_dest);
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //logic to encode the data.
    // run operation upto 8 times (char 8times)
    for(int i = 0;i < 8;i++)
    {  
        image_buffer[i] = image_buffer[i] & 0xFE;
        image_buffer[i] = image_buffer[i] | ((data >> (7 - i)) & 1);
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i = 0;i < 32;i++)
    {
        imageBuffer[i] = imageBuffer[i] & 0xFE;
        imageBuffer[i] = imageBuffer[i] | ((size >> (31 - i)) & 1);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //step1 -> call open file(encInfo)
    if(open_files(encInfo) == e_failure)
    {
        // e_failure -> print error msg and return e_failure
        return e_failure;
    }
    // e_success -> print success msg goto next step
    printf("Done\n");

    printf("## Encoding Procedure Started ##\n");
    // step2 -> call check_capacity(encInfo)
    if(check_capacity(encInfo) == e_failure)
    {
        // e_failure -> print error msg and return e_failure
        printf("Error : source file doesnt have capacity\n");
        return e_failure;
    }
    // e_success -> print success msg goto next step
    printf("Done\n");
    
    // step 3 -> call copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr->stego_image)
    if(copy_bmp_header(encInfo ->fptr_src_image,encInfo ->fptr_stego_image) == e_failure)
    {
        printf("Error : unable to copy heade file\n");
        return e_failure;
    }
	//print success msg and goto next step
    printf("Done\n");

	// step4 -> call magic string(MA.GIC_STRING,encInfo)
    if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure)
    {
        printf("Error : unable to encode magic string\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step5 ->call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file),encInfo)
    if(encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file),encInfo) == e_failure)
    {
        printf("Error : unable to encode secret file extn size\n");
        return e_failure;
    }
	//print success msg to go to next step
    printf("Done\n");

	// step6 ->call encode_secret_file_extna(encInfo->extn_secret_file,encInfo)
    if(encode_secret_file_extn(encInfo -> extn_secret_file,encInfo) == e_failure)
    {
        printf("Error : unable to encode secret file extn\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step7 -> call encode_secret_file_size(encInfo -> size_secret_file,encInfo)
    if(encode_secret_file_size(encInfo -> size_secret_file,encInfo) == e_failure)
    {
        printf("Error : unable to encode secret file size\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");
	// step8 -> call encode_secret_file_data(encInfo)

    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Error : unable to encode secret file data\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step9 -> call copy_remaining_img_data(src filepointer,dest filepointer)
    if(copy_remaining_img_data(encInfo ->fptr_src_image,encInfo ->fptr_stego_image) == e_failure)
    {
        printf("Error : unable to copy remaining data\n");
        return e_failure;
    }
	// print success msg and goto next step
    printf("Done\n");

	// step10 -> retrun e_success
    return e_success;
}
