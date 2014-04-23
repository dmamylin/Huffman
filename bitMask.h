#ifndef BIT_MASK_H
#define BIT_MASK_H

#include "types.h"

#define BITS_IN_BYTE 8

typedef struct BitMask {
    int pos; //filled position with the biggest index (from left to right)
    u8  data; //data stored from left to right
} BitMask;

void bitMaskInit(BitMask* mask);
void bitMaskAdd(BitMask* mask, u8 val);
u8   bitMaskGet(BitMask* mask, int i);
u8   bitMaskPop(BitMask* mask);
int  bitMaskIsFull(BitMask* mask);
int  bitMaskIsEmpty(BitMask* mask);

#endif