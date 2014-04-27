#ifndef DECODER_H
#define DECODER_H

#include <stdio.h>

#include "types.h"
#include "stack.h"
#include "btree.h"
#include "bitMask.h"

int  loadFromFile(FILE* fileIn);
void treeFromFile(FILE* fileIn, BinTree** tree);
void decodeFile(FILE* fileIn, FILE* fileOut, u64 fileLen, const BinTree* root);

#endif