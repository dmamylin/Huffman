#include "decoder.h"

/*uncompress compressed fileIn. If name != NULL, unpacked file's name will be "name"
result - TRUE or FALSE*/
int loadFromFile(FILE* fileIn, const char* name) {
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
    fread((void*)str, sizeof(s8), info.nameLen, fileIn); //read file name
    str[info.nameLen] = '\0';
    fileOut = fopen(name ? name : str, "wt"); //and open file
    free(str);

    if ( !fileOut ) {
        return FALSE;
    }

    fgetpos(fileIn, &pos); //save position of data blocks
    fseek(fileIn, sizeof(u8) * info.blocksCount, SEEK_CUR); //go to the tree's data
    treeFromFile(fileIn, &root); //read tree
    fsetpos(fileIn, &pos); // return to data's position

    decodeFile(fileIn, fileOut, info.fileLen, root); //decoding

    fclose(fileOut); //cleanup allocated memory
    binTreeRemove(root);

    return TRUE;
}

//get tree from file
void treeFromFile(FILE* fileIn, BinTree** tree) {
    u8 byte[2];

    //get 2 bytes from fileIn: [byte1][byte2]
    //byte[0] - data (arbitrary for non-leaf, symbols for leafs)
    //byte[1] - flag: 0 == not a leaf; 1 == leaf
    fread((void*)byte, sizeof(s8), 2, fileIn);

    binTreeInit(tree);
    (*tree)->key.val = (u32)byte[0];

    //recursively go through file
    if ( byte[1] == (u8)0 ) {
        treeFromFile(fileIn, &(*tree)->left);
        treeFromFile(fileIn, &(*tree)->right);
    }
}

//decode bits from file "fileIn" and save to the "fileOut" according fileLen and tree
void decodeFile(FILE* fileIn, FILE* fileOut, u64 fileLen, const BinTree* root) {
    BitMask        mask;
    const BinTree* temp = root;

    bitMaskInit(&mask);

    while ( fileLen > 0 ) {
        int j;

        fread((void*)&mask.data, sizeof(u8), 1, fileIn); //read 1 byte
        mask.pos  = 7; //set bit mask position on the most significant bit

        //for bits count or while fileLen > 0
        for ( j = 0; j < 8 && fileLen > 0; j++ ) {
            u8 bit = bitMaskGet(&mask, j); //get bit from mask

            temp = bit == (u8)0 ? temp->left : temp->right; //determine path in tree

            if ( binTreeIsLeaf(temp) ) { //write data (symbol) of each leaf to "fileOut"
                fwrite((const void*)&temp->key.val, sizeof(s8), 1, fileOut);
                temp = root;
                fileLen--;
            }
        }
    }
}