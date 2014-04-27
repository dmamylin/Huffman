#include "decoder.h"

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