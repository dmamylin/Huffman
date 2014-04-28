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
void printHelp();

//compare symbols s1 and s2. Uses in quick sort function
int compareSym(const void* s1, const void* s2) {
    return (*(Symbol**)s1)->freq - (*(Symbol**)s2)->freq;
}

//init symbols and symbolsOrd arrays
void initializeArrays(Symbol* symbols, Symbol** symbolsOrd, int len) {
    int i;

    for ( i = 0; i < len; i++ ) {
        symbols[i].sym  = (char)i;
        symbols[i].freq = 0;
        symbolsOrd[i]   = &symbols[i];
        memset(symbols[i].path, '\0', sizeof(symbols[i].path));
    }
}

void printHelp() {
    puts("-e (--encode) f1 f2 - compress file named f1 into archive named f2");
    puts("-d (--decode) f1 [f2] - decompress archive named f1 [optional: into file named f2]");
    puts("\tif name not specified, takes it from archive.");
}

int main(int argc, char** argv) {
    FILE*   fileIn  = NULL;
    FILE*   fileOut = NULL;
    Symbol  symbols[ARRAY_LENGTH];
    Symbol* symbolsOrd[ARRAY_LENGTH]; //ordered pointers (by frequency)

    u64 fileLen = 0;

    BinTree* temp[2];
    BinTree* root;
    Stack    stack;

    if ( !argv[1] ) {
        printHelp();
        return EXIT_FAIL;
    }

    if ( !strcmp(argv[1], "-e") || !strcmp("--encode", argv[1]) ) {
        if ( !argv[2] || !argv[3] ) {
            printHelp();
            return EXIT_FAIL;
        }

        fileIn  = (FILE*)fopen(argv[2], "rt");
        if ( !fileIn ) {
            printf("Error: file <<%s>> doesn't exists\n", argv[2]);
            return EXIT_FAIL;
        }

        stackInit(&stack);
        initializeArrays(symbols, symbolsOrd, ARRAY_LENGTH);

        fileLen = readFile(fileIn, symbols); //read symbols; first pass
        if ( fileLen == 0 ) {
            printf("Error: file <<%s>> is empty\n", argv[2]);
            fclose(fileIn);
            return EXIT_FAIL;
        }

        fileOut = (FILE*)fopen(argv[3], "wb");
        if ( !fileOut ) {
            printf("Error: cannot create file <<%s>>\n", argv[3]);
            fclose(fileIn);
            return EXIT_FAIL;
        }

        //sort symbols by frequencies from lower to greatest
        qsort(symbolsOrd, ARRAY_LENGTH, sizeof(Symbol*), compareSym);
        fillStack(symbolsOrd, &stack, ARRAY_LENGTH); //prepare stack

        root = buildTree(&stack);

        createPath(root, 0); //0 - starting depth

        rewind(fileIn); //reopen fileIn; prepare second pass

        //write all compressed data
        saveToFile(fileIn, fileOut, symbols, root, argv[2], fileLen);

        //memory cleanup
        binTreeRemove(root);

        fclose(fileOut);
        fclose(fileIn);
    } else if ( !strcmp(argv[1], "-d") || !strcmp("--decode", argv[1]) ) {
        if ( !argv[2] ) {
            printHelp();
            return EXIT_FAIL;
        }

        fileIn = fopen(argv[2], "rb");
        if ( !fileIn ) {
            printf("Error: file <<%s>> doesn't exists\n", argv[2]);
            return EXIT_FAIL;
        }

        if ( !loadFromFile(fileIn, argv[3] ? argv[3] : NULL) ) {
            printf("Error: file <<%s>> corrupted or cannot create file <<%s>>\n",
                argv[2], argv[3]);
            fclose(fileIn);
            return EXIT_FAIL;
        }

        fclose(fileIn);
    } else {
        printHelp();
    }
    

    return EXIT_SUCCESS;
}