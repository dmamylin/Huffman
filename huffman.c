#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "btree.h"
#include "stack.h"
#include "bitMask.h"

int  compareSym(const void*, const void*);

void readFile(FILE* file, Symbol* symbols, u64* fileLen);
void saveToFile(FILE* fileIn, FILE* fileOut, Symbol* symbols, BinTree* tree,
              const char* name, u64 len);
int  loadFromFile(FILE* fileIn);

void pushOrdered(Stack* stack, BinTree* newNode);
void createPath(BinTree* tree, int depth);
void treeToFile(FILE* fileOut, BinTree* tree);
void treeFromFile(FILE* fileIn, BinTree** tree);
u32  encodeFile(FILE* fileIn, FILE* fileOut, Symbol* symbols); //converting tree to binary sequence
void decodeFile(FILE* fileIn, FILE* fileOut, u64 fileLen, const BinTree* root);

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

void saveToFile(FILE* fileIn, FILE* fileOut, Symbol* symbols, BinTree* tree,
              const char* name, u64 len) {
    FileInfo info;
    const char pref[] = FILE_PREFIX;
    fpos_t pos;

    memcpy((void*)info.prefix, pref, sizeof(s8) * 4);
    info.fileLen     = len;
    info.blocksCount = 0;
    info.nameLen     = strlen(name);

    fwrite((const void*)&info, sizeof(FileInfo), 1, fileOut); //write info prefix
    fwrite((const void*)name, sizeof(char), info.nameLen, fileOut); //write filename

    info.blocksCount = encodeFile(fileIn, fileOut, symbols);

    fgetpos(fileOut, &pos); //get current position
    rewind(fileOut);

    fwrite((const void*)&info, sizeof(FileInfo), 1, fileOut); //write updated info

    fsetpos(fileOut, &pos); //restore position
    //now, file format is: [FILE_INFO],[FILE_NAME],[DATA],position
    //time to save the tree

    treeToFile(fileOut, tree);
}

int loadFromFile(FILE* fileIn) {
    FileInfo   info;
    fpos_t     pos;
    BinTree*   root = NULL;
    const char pref[] = FILE_PREFIX;
    FILE*      fileOut;
    char*      str;

    fread(&info, sizeof(FileInfo), 1, fileIn); //get prefix

    if ( memcmp(pref, &info.prefix, sizeof(s8) * 4) ) {
        return FALSE; //check that 4-byte prefix == "HUFF"
    }

    str = (char*)malloc((info.nameLen + 1) * sizeof(s8));
    fread((void*)str, sizeof(s8), info.nameLen, fileIn); //get file name
    str[info.nameLen] = '\0';
    str[0] = '!'; //TODODODODODODODODODPDODODOD!!!DODOD!O!O!
    fileOut = fopen(str, "wt"); //and open file
    free(str);

    if ( !fileOut ) {
        return FALSE;
    }

    fgetpos(fileIn, &pos); //save data's position
    fseek(fileIn, sizeof(u8) * info.blocksCount, SEEK_CUR); //go to the tree's data
    treeFromFile(fileIn, &root);
    fsetpos(fileIn, &pos); // return to data

    decodeFile(fileIn, fileOut, info.fileLen, root);

    fclose(fileOut);
    binTreeRemove(root);

    return TRUE;
}

void pushOrdered(Stack* stack, BinTree* newNode) {
    BinTree* oldNode = NULL;
    u32      freq    = 0;

    if ( !stackPop(stack, (void**)&oldNode) ) {
        stackPush(stack, (void*)newNode);
        return;
    }

    //get frequency of the top element in the stack
    freq = binTreeIsLeaf(oldNode) ? ((Symbol*)oldNode->key.ptr)->freq
        : oldNode->key.val;

    if ( newNode->key.val <= freq ) { //if newFreq <= oldFreq
        stackPush(stack, oldNode);
        stackPush(stack, newNode);
        return; //push them with right order and stop
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

void treeToFile(FILE* fileOut, BinTree* tree) {
    //TODO: maybe 9-byte?
    //tree format: [data][flag]; data & flag - 1 byte blocks
    //data: data in node;
    //flag: 0 => not a leaf; 1 => leaf

    if ( tree ) { //works only on non-empty nodes
        if ( binTreeIsLeaf(tree) ) {
            u8 buff = 1;

            //save data:
            fwrite((const void*)&((Symbol*)tree->key.ptr)->sym, sizeof(char), 1, fileOut);
            fwrite((const void*)&buff, sizeof(u8), 1, fileOut); //save flag
        } else {
            u8 buff = 0;

            //save arbitrary data and 0 flag:
            fwrite((const void*)&buff, sizeof(u8), 1, fileOut);
            fwrite((const void*)&buff, sizeof(u8), 1, fileOut); 
            //fwrite((const void*)&buff, sizeof(u8), 2, fileOut); WTF!?

            treeToFile(fileOut, tree->left);
            treeToFile(fileOut, tree->right);
        }
    }
}

void treeFromFile(FILE* fileIn, BinTree** tree) {
    u8 byte[2];

    fread((void*)&byte[0], sizeof(s8), 1, fileIn);
    /*if ( byte[0] == EOF ) {
        return;
    }*/
    fread((void*)&byte[1], sizeof(s8), 1, fileIn);

    binTreeInit(tree);
    (*tree)->key.val = (u32)byte[0];

    if ( byte[1] == (u8)0 ) {
        treeFromFile(fileIn, &(*tree)->left);
        treeFromFile(fileIn, &(*tree)->right);
    }
}

u32 encodeFile(FILE* fileIn, FILE* fileOut, Symbol* symbols) {
    //symbols[] - array of symbols from fileIn with filled "path" fields
    BitMask mask;
    int     i;
    u32     count = 0; //byte-blocks count

    bitMaskInit(&mask);
    while ( (i = fgetc(fileIn)) != EOF ) {
        char* j = symbols[i].path;

        while ( *j != '\0' ) {
            if ( bitMaskIsFull(&mask) ) {
                //if byte block is filled, writing it to file
                fwrite((const void*)&mask.data, sizeof(u8), 1, fileOut);
                bitMaskInit(&mask);
                count++;
            }

            bitMaskAdd(&mask, (u8)(*j == '0' ? 0 : 1));
            j++;
        }
    }
    fwrite((const void*)&mask.data, sizeof(u8), 1, fileOut); //save last block
    count++;

    return count;
}

void decodeFile(FILE* fileIn, FILE* fileOut, u64 fileLen, const BinTree* root) {
    BitMask   mask;
    const BinTree*  temp = root;

    bitMaskInit(&mask);

    while ( fileLen > 0 ) {
        int j;

        fread((void*)&mask.data, sizeof(u8), 1, fileIn);
        mask.pos  = 7;
        for ( j = 0; j < 8 && fileLen > 0; j++ ) {
            u8 tmp = bitMaskGet(&mask, j);

            temp = tmp == (u8)0 ? temp->left : temp->right;

            if ( binTreeIsLeaf(temp) ) {
                fwrite((const void*)&temp->key.val, sizeof(s8), 1, fileOut);
                temp = root;
                fileLen--;
            }
        }
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

    for ( i = 0; i < ARRAY_LENGTH; i++ ) {
        symbols[i].sym  = (char)i;
        symbols[i].freq = 0;
        symbolsOrd[i]   = &symbols[i];
        memset(symbols[i].path, '\0', sizeof(symbols[i].path));
    }

    readFile(fileIn, symbols, &fileLen);

    qsort(symbolsOrd, ARRAY_LENGTH, sizeof(Symbol*), compareSym);

    //converting each symbol to the tree's leaf and pushing it to stack
    //back order - right order
    for ( i = ARRAY_LENGTH - 1; i >= 0; i-- ) {
        temp[0] = NULL;

        if ( symbolsOrd[i]->freq <= 0 ) {
            break; //stop when zero frequencies appear
        }

        binTreeInit(&temp[0]);
        temp[0]->key.ptr = (void*)symbolsOrd[i];

        stackPush(&stack, (void*)temp[0]);
    } //stack will be filled from lessers(on top) to greatests

    //popping 2 elements from stack, if it possible
    //TODO: handle 1 element stacks and empty stacks (!!!!!!!)
    while ( stackPop(&stack, (void**)temp) &&
            stackPop(&stack, (void**)(temp+1)) ) {
        BinTree* newNode = NULL;

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

    rewind(fileIn);
    saveToFile(fileIn, fileOut, symbols, root, argv[2], fileLen);

    fclose(fileOut);
    fileOut = fopen(argv[2], "rb");
    loadFromFile(fileOut);

    binTreeRemove(root);

    fclose(fileIn);
    fclose(fileOut);
    return EXIT_SUCCESS;
}
