#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "btree.h"
#include "stack.h"
#include "bitMask.h"

#include "encoder.h"
#include "decoder.h"

void initializeArrays(Symbol* symbols, Symbol** symbolsOrd, int len);
int compareSym(const void*, const void*);

int compareSym(const void* s1, const void* s2) {
    return (*(Symbol**)s1)->freq - (*(Symbol**)s2)->freq;
}

void initializeArrays(Symbol* symbols, Symbol** symbolsOrd, int len) {
    int i;

    for ( i = 0; i < len; i++ ) {
        symbols[i].sym  = (char)i;
        symbols[i].freq = 0;
        symbolsOrd[i]   = &symbols[i];
        memset(symbols[i].path, '\0', sizeof(symbols[i].path));
    }
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
    initializeArrays(symbols, symbolsOrd, ARRAY_LENGTH);
    fileLen = readFile(fileIn, symbols);
    qsort(symbolsOrd, ARRAY_LENGTH, sizeof(Symbol*), compareSym);

    fillStack(symbolsOrd, &stack, ARRAY_LENGTH);

    root = buildTree(&stack);

    createPath(root, 0); //0 - starting depth

    rewind(fileIn);
    saveToFile(fileIn, fileOut, symbols, root, argv[1], fileLen);

    fclose(fileOut);
    fileOut = fopen(argv[2], "rb");
    loadFromFile(fileOut);

    binTreeRemove(root);

    fclose(fileIn);
    fclose(fileOut);
    return EXIT_SUCCESS;
}