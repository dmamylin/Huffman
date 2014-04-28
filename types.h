#ifndef TYPES_H
#define TYPES_H

#define FALSE 0
#define TRUE  1

#define EXIT_SUCCESS	0
#define EXIT_FAIL		1

#define ARRAY_LENGTH    256 //number of unique symbols (in ASCII table particularly)
#define FILE_PREFIX     "HUFF"

typedef char  s8;
typedef short s16;
typedef int   s32;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef struct {
    u32   freq; //frequency
    char  sym;  //symbol
    //path in the binary tree - null-treminated string of 1's and 0's
    char  path[ARRAY_LENGTH + 1];
} Symbol;

typedef struct {
    s8  prefix[4]; //must be "HUFF" in the right output (encoded) file
    u64 fileLen; //in symbols
    u32 blocksCount; //number of byte-blocks of data
    u16 nameLen; //length of input file's name
} FileInfo;

#endif