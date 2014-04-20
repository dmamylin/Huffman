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
int  binTreeIsLeaf(BinTree*  root);

void binTreeRemove(BinTree* root) {
	if ( root ) { //remove if root != NULL. If root == NULL - already removed.
		binTreeRemove(root->left); //recursively remove left and right brunches
		binTreeRemove(root->right);
		free(root); //and kill root
	}
}

int binTreeInit(BinTree** root) {
	if ( *root ) { //if root already allocated
		return TRUE;
	}

	*root = (BinTree*)malloc(sizeof(BinTree));
	if ( !(*root) ) { //allocation problem
		return FALSE;
	}

	(*root)->left    = (*root)->right = NULL;
    (*root)->key.ptr = NULL; 
    
	return TRUE;
}

int binTreeIsLeaf(BinTree* root) {
	return !root->left && !root->right;
}

#endif