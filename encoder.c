#include "encoder.h"

/*fileIn - original file
  fileOut - output (compressed file)
  symbols - filled array of symbols with their frequencies and path
  tree - filled Huffman's tree
  name - name that will be stored in archive
  len - lenght of file in bytes
compressed file format: 
[HUFF-prefix (4bytes)], [file length (8bytes)], [data blocks count (4bytes)],
[length of filename (2bytes)], [filename], [dataBlocks], [treeData]*/
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

/*read file and save frequencies in array of symbols.
result - length of file in bytes
file (stream) position is modifiable (!!!)*/
u64 readFile(FILE* file, Symbol* symbols) {
    int ch = 0;
    u64 len = 0;

    while ( (ch = fgetc(file)) != EOF ) {
        symbols[ch].freq++;
        len++;
    }

    return len;
}

/*for each symbol from "fileIn" compress path into bit sequence and save it to 
the "fileOut", where "symbols" - filled array of symbols with their frequencies and path
result - number of written bytes
file (stream) position is modifiable (!!!)*/
u32 encodeFile(FILE* fileIn, FILE* fileOut, Symbol* symbols) {
    //symbols[] - array of symbols from fileIn with filled "path" fields
    BitMask mask;
    int     i;
    u32     count = 0; //writed bytes count

    bitMaskInit(&mask);
    while ( (i = fgetc(fileIn)) != EOF ) { //for each symbol in fileIn
        char* j = symbols[i].path; //path - null-terminated string of 1's and 0's

        while ( *j != '\0' ) { 
            if ( bitMaskIsFull(&mask) ) { //if byte block is filled, write it to file
                fwrite((const void*)&mask.data, sizeof(u8), 1, fileOut);
                bitMaskInit(&mask); //clear byte block
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

//create binary path to symbols in tree
void createPath(BinTree* tree, int depth) {
    //recursion + static variable = powerful trick
    static char temp[ARRAY_LENGTH]; //temporary path

    if ( binTreeIsLeaf(tree) ) {
        //if leaf had been appeared, saving accumulated path
        memcpy(&((Symbol*)tree->key.ptr)->path, temp, depth * sizeof(char));
        return;
    }

    //repeat recursively
    temp[depth] = '0'; //0 - left branch
    createPath(tree->left, depth + 1);

    temp[depth] = '1'; //1 - right branch
    createPath(tree->right, depth + 1);
}

/*save binary tree to the file
file (stream) position is modifiable (!!!)*/
void treeToFile(FILE* fileOut, BinTree* tree) {
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

            //save arbitrary data and 0 (not a leaf) flag:
            fwrite((const void*)&buff, sizeof(u8), 1, fileOut); //data
            fwrite((const void*)&buff, sizeof(u8), 1, fileOut); //flag

            //repeat recursively
            treeToFile(fileOut, tree->left);
            treeToFile(fileOut, tree->right);
        }
    }
}

//convert each symbol with non-zero frequency into tree's leaf and push it to stack
void fillStack(Symbol** symbolsOrd, Stack* stack, int len) {
    int i;
    BinTree* temp;

    for ( i = len - 1; i >= 0; i-- ) {
        temp = NULL;

        if ( symbolsOrd[i]->freq <= 0 ) {
            break; //stop when zero frequencies appear
        }

        binTreeInit(&temp);
        temp->key.ptr = (void*)symbolsOrd[i];

        stackPush(stack, (void*)temp);
    } //stack will be filled from lessers(on top) to greatests
}

/*build tree by stack of leafs. 
result - pointer on the new tree*/
BinTree* buildTree(Stack* stack) {
    BinTree* temp[2];

    temp[0] = temp[1] = NULL;

    //pop 2 elements from stack, while possible
    while ( stackPop(stack, (void**)temp) &&
            stackPop(stack, (void**)(temp+1)) ) {
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
        pushOrdered(stack, newNode);
    }

    return temp[0]; //result (tree pointer) in temp[0]
}

/*push node of bin tree to the stack and restore order (from lesser on top to
greatest in bottom)*/
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