#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "btree.h"
#include "stack.h"
#include "bitMask.h"

#define ARRAY_LENGTH    256

typedef struct {
    u32   freq; //frequency
    char  sym;
    char  path[ARRAY_LENGTH + 1];
} Symbol;

int  compareSym(const void*, const void*);
void readFile(FILE* file, Symbol* symbols, u64* fileLen);
void pushOrdered(Stack* stack, BinTree* newNode);
void createPath(BinTree* tree, int depth);

int compareSym(const void* s1, const void* s2) {
    return (*(Symbol**)s1)->freq - (*(Symbol**)s2)->freq;
}

void readFile(FILE* file, Symbol* symbols, u64* fileLen) {
    int ch = 0;

    while ( (ch = fgetc(file)) != EOF ) {
        symbols[ch].freq++;
        (*fileLen)++;
    }
}

void pushOrdered(Stack* stack, BinTree* newNode) {
    BinTree* oldNode = NULL;
    u32      freq    = 0;

    if ( !stackPop(stack, (void**)&oldNode) ) {
        stackPush(stack, (void*)newNode);
        return;
    }

    //get frequence of the top element in the stack
    freq = binTreeIsLeaf(oldNode) ? ((Symbol*)oldNode->key.ptr)->freq
        : oldNode->key.val;

    if ( newNode->key.val <= freq ) { //if newFreq <= oldFreq
        stackPush(stack, oldNode);
        stackPush(stack, newNode);
        return; //push them in right order and stop
    } else {
        pushOrdered(stack, newNode); //push again to popped stack
        stackPush(stack, oldNode); //and push oldNode
    }
}

void createPath(BinTree* tree, int depth) { //create binary path to symbols
    static char temp[ARRAY_LENGTH]; //temporary path

    if ( binTreeIsLeaf(tree) ) {
        memcpy(&((Symbol*)tree->key.ptr)->path, temp, depth * sizeof(char));
        return;
    }

    temp[depth] = '0';
    createPath(tree->left, depth + 1);

    temp[depth] = '1';
    createPath(tree->right, depth + 1);
}

int main(int argc, char** argv) {
    FILE*   fileIn  = NULL;
    FILE*   fileOut = NULL;
    Symbol  symbols[ARRAY_LENGTH];
    Symbol* symbolsOrd[ARRAY_LENGTH]; //ordered pointers (by frequency)

    int i;
    u64 fileLen = 0;

    BinTree* temp[2];
    BinTree* root;
    Stack    stack;
    BitMask  mask;
    
    if ( argc < 3 ) {
        puts("Error: too few arguments.");
        return EXIT_FAIL;
    }

    fileIn  = (FILE*)fopen(argv[1], "rt");
    fileOut = (FILE*)fopen(argv[2], "wb");

    if ( !fileIn || !fileOut ) {
        fclose(fileIn);
        fclose(fileOut);
        printf("Error: file %s or %s corrupted or doesn't exist.", 
            argv[1], argv[2]);

        return EXIT_FAIL;
    }
    
    stackInit(&stack);    

    for ( i = 0; i < ARRAY_LENGTH; i++ ) {
        symbols[i].sym  = (char)i;
        symbols[i].freq = 0;
        symbolsOrd[i]   = &symbols[i];
        memset(symbols[i].path, '\0', sizeof(symbols[i].path));
    }

    readFile(fileIn, symbols, &fileLen);

    qsort(symbolsOrd, ARRAY_LENGTH, sizeof(Symbol*), compareSym);

    //back order - to fill the stack
    for ( i = ARRAY_LENGTH - 1; i >= 0; i-- ) {
        temp[0] = NULL;

        if ( symbolsOrd[i]->freq <= 0 ) {
            break; //stop when non-zero frequencies appear
        }

        binTreeInit(&temp[0]);
        temp[0]->key.ptr = (void*)symbolsOrd[i];
        temp[0]->left = temp[0]->right = NULL;

        stackPush(&stack, (void*)temp[0]);
    } //stack will be filled from lessers(on top) to greatests

    //pushing 2 elements from stack, if it possible
    //TODO: handle 1 element stacks and empty stacks (!!!!!!!)
    while ( stackPop(&stack, (void**)temp) &&
            stackPop(&stack, (void**)(temp+1)) ) {
        BinTree* newNode = NULL;
        BinTree* oldNode = NULL;

        //and merge them into new node
        binTreeInit(&newNode);
        newNode->left    = temp[0];
        newNode->right   = temp[1];
        newNode->key.val = 0;

        //add left's frequencies
        if ( binTreeIsLeaf(newNode->left) ) {
            newNode->key.val += ((Symbol*)newNode->left->key.ptr)->freq;
        } else {
            newNode->key.val += newNode->left->key.val;
        }
        
        //add right's frequencies
        if ( binTreeIsLeaf(newNode->right) ) {
            newNode->key.val += ((Symbol*)newNode->right->key.ptr)->freq;
        } else {
            newNode->key.val += newNode->right->key.val;
        }

        //push newNode to stack and restore order
        pushOrdered(&stack, newNode);
    }

    root = temp[0]; //result tree in temp[0]
    
    createPath(root, 0);
    
    fseek(fileIn, 0, SEEK_SET);
    bitMaskInit(&mask);
    while ( (i = fgetc(fileIn)) != EOF ) {
        char* j = symbols[i].path;

        while ( *j != '\0' ) {
            if ( bitMaskIsFull(&mask) ) {
                //fprintf(fileOut, "%c", (char)mask.data);
                fwrite((const void*)&mask.data, sizeof(u8), 1, fileOut);
                bitMaskInit(&mask);
            }

            bitMaskAdd(&mask, (u8)(*j == '0' ? 0 : 1));
            j++;
        }
    }
    if ( !bitMaskIsEmpty(&mask) ) {
        fprintf(fileOut, "%c", (char)mask.data);
    }
    
    bitMaskInit(&mask);
    fclose(fileOut);
    fclose(fileIn);
    fileOut = fopen(argv[2], "rb");
    fileIn  = fopen("temp", "wt");
    temp[0] = root;
    
    while ( fileLen > 0 ) {        
        int j;

        fread((void*)&mask.data, sizeof(u8), 1, fileOut);
        mask.pos  = 7;
        for ( j = 0; j < 8 && fileLen > 0; j++ ) {
            u8 tmp = bitMaskGet(&mask, j);

            temp[0] = tmp == (u8)0 ? temp[0]->left : temp[0]->right;

            if ( binTreeIsLeaf(temp[0]) ) {
                fprintf(fileIn, "%c", ((Symbol*)temp[0]->key.ptr)->sym);
                //fwrite((const void*)&mask.data, sizeof(u8), 1, temp);
                temp[0] = root;
                fileLen--;
            }
        }
    }
    
    binTreeRemove(root);
    
    fclose(fileIn);
    fclose(fileOut);
    return EXIT_SUCCESS;
}