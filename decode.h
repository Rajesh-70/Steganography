#ifndef DECODE_H
#define DECODE_H

#include<stdio.h>
#include"types.h"

typedef struct _DecodeInfo
{
    // Stego Image Info 
    char *stego_image_fname; 
    FILE *fptr_stego_image;    

    // Secret File Info 
    char output_fname[100];       
    FILE *fptr_secret;
    int secret_extn_size;
    long size_secret_file; 

} DecodeInfo;

// Decoding function prototype 

// Read and validate decode args from argv 
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

// Perform the decoding 
Status do_decoding(DecodeInfo *decInfo);

// Get File pointers for i/p and o/p files 
Status open_decode_files(DecodeInfo *decInfo);

// Decode Magic String 
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

//decode extension size
Status decode_secret_file_extn_size(int *size,DecodeInfo *decInfo);

// decode secret file extenstion 
Status decode_secret_file_extn(DecodeInfo *decInfo);

// decode secret file size 
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

// decode secret file data
Status decode_secret_file_data(DecodeInfo *decInfo);

// decode a byte into LSB of image data array 
Status decode_byte_to_lsb(char *data, char *image_buffer);

// decode a size to lsb
Status decode_size_to_lsb(int *size, char *imageBuffer);

#endif