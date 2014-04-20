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

void stackInit(Stack* s) {
	s->head  = -1; //empty by default
}

int stackPush(Stack* s, void* data) {
	if ( (s->head + 1) >= SIZE ) {
		return FALSE; //stack overflow 
	}

	s->head++;
	s->data[s->head] = data;

	return TRUE;
}

int stackPop(Stack* s, void** src) {
	if ( s->head < 0 ) {
		return FALSE; //stack is empty
	}

	*src = s->data[s->head];
	s->head--;

	return TRUE;
}

#undef SIZE

#endif