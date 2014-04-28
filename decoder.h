#ifndef DECODER_H
#define DECODER_H

#include <stdio.h>

#include "types.h"
#include "stack.h"
#include "btree.h"
#include "bitMask.h"

//uncompress compressed fileIn. If name != NULL, unpacked name will be "name"
int  loadFromFile(FILE* fileIn, const char* name);

//get tree from file fileIn
void treeFromFile(FILE* fileIn, BinTree** tree);

//bit decoding from file "fileIn" and saving to the "fileOut" according fileLen and tree
void decodeFile(FILE* fileIn, FILE* fileOut, u64 fileLen, const BinTree* root);

#endif