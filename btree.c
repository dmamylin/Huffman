#include "btree.h"

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

int binTreeIsLeaf(const BinTree* root) {
    return root && !root->left && !root->right;
}