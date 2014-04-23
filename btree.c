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

#include <stdio.h>
void binTreePrint(const BinTree* root, s32 inChar) {
    if ( binTreeIsLeaf(root) ) {
        inChar ? printf(" <%c> ", (char)root->key.val) :
                 printf(" <%u> ", (u32)root->key.val);
    } else {
        printf(" %s ", "<NODE>");
        binTreePrint(root->left, inChar);
        binTreePrint(root->right, inChar);
    }
}