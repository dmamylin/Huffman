#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <string.h>

#include "types.h"

#define SIZE  256

//stack with fixed length SIZE
typedef struct {
	void*  data[SIZE];
	s32    head; //top of stack (index). Head == -1 <=> stack is empty
} Stack;

void stackInit(Stack* s);
int  stackPush(Stack* s, void* data);
int  stackPop (Stack* s, void** src);

#endif