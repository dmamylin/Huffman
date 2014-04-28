#include "bitMask.h"

void bitMaskInit(BitMask* mask) {
    mask->pos  = -1; //last filled position; position < 0 indicates empty mask
    mask->data = (u8)0; //00000000 by default
}

void bitMaskAdd(BitMask* mask, u8 val) { //set current bit to val's state
    int i;
    u8  temp = (u8)(val > 0 ? 1 : 0);

    mask->pos++;

    i = sizeof(mask->data) * BITS_IN_BYTE - 1 - mask->pos; //position of bit
    temp <<= i; //shift: 0..01 -> 0..1..0
    mask->data |= temp; //put new bit to the data
}

u8 bitMaskGet(BitMask* mask, int i) { //get bit by current position
    u8  temp = (u8)1;

    i = sizeof(mask->data) * BITS_IN_BYTE - 1 - i;
    temp <<= i;
    temp &= mask->data;

    return (u8)(temp > (u8)0 ? 1 : 0);
}

int bitMaskIsFull(BitMask* mask) {
    return mask->pos + 1 >= (int)(sizeof(mask->data) * BITS_IN_BYTE);
}

int bitMaskIsEmpty(BitMask* mask) {
    return mask->pos < 0;
}

#undef BITS_IN_BYTE