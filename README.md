# Image Steganography in C

## Description
This project implements **Image Steganography** using the **Least Significant Bit (LSB)** technique in C.  
It allows users to **hide and extract secret messages inside BMP image files** while maintaining minimal visual distortion.  
The project demonstrates binary file handling, bitwise operations, and basic information security concepts.

---

## Features
- Encode secret text into BMP images
- Decode hidden messages from BMP images
- LSB (Least Significant Bit) data hiding technique
- Minimal visual distortion of original image
- Efficient binary file handling
- Modular program structure

---

## Technologies Used
- C Programming
- GCC Compiler
- File I/O Operations (`fopen`, `fread`, `fwrite`, `fseek`)
- Bitwise Manipulation
- Structures and Pointers
- Linux / Windows Environment

---

## Project Structure
```text
Image-Steganography/
│
├── main.c
├── encode.c
├── decode.c
├── common.h
├── encode.h
├── decode.h
└── README.md
---

## How It Works
1. Reads BMP image header and pixel data.
2. Converts secret message into binary form.
3. Embeds message bits into the LSB of image pixels.
4. Saves the encoded image as a new BMP file.
5. Decoding reverses the process to retrieve the hidden message.

---

## How to Compile

```bash
gcc main.c encode.c decode.c -o stego
