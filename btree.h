#ifndef BTREE_H
#define BTREE_H

#include <stdlib.h>

#include "types.h"

typedef union BinKey {
    void* ptr;
    u32   val;
} BinKey;

typedef struct BinTree {
	BinKey key;
	struct BinTree* left;
	struct BinTree* right;
} BinTree;

void binTreeRemove(BinTree*  root);
int  binTreeInit  (BinTree** root);
int  binTreeIsLeaf(const BinTree*  root);
void binTreePrint(const BinTree* root, s32 inChar);

#endif
