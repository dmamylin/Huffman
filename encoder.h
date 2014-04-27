#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>

#include "types.h"
#include "stack.h"
#include "btree.h"
#include "bitMask.h"

//reading file and saving frequencies in array of symbols.
//result - length of fyle in bytes
u64 readFile(FILE* file, Symbol* symbols);

void saveToFile(FILE* fileIn, FILE* fileOut, Symbol* symbols, BinTree* tree,
              const char* name, u64 len);

/*pushing node of bin tree to the stack and restore order (from lesser on top to
greatest in bottom)*/
void pushOrdered(Stack* stack, BinTree* newNode);

/*determine path in tree (0 - left branch, 1 - right branch) and store it in the
array of symbols*/
void createPath(BinTree* tree, int depth);

//save tree to given file "fileOut"
void treeToFile(FILE* fileOut, BinTree* tree);

//compressing "fileIn" to the "fileOut" according "symbols" array
u32 encodeFile(FILE* fileIn, FILE* fileOut, Symbol* symbols);

//converting each symbol with non-zero frequency into tree's leaf and pushing it to stack
void fillStack(Symbol** symbolsOrd, Stack* stack, int len);

//building tree by stack of leafs. Result - pointer on a new tree
BinTree* buildTree(Stack* stack);

#endif