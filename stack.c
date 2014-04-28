#include "stack.h"

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