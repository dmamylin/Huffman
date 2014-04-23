#include "bitMask.h"

void bitMaskInit(BitMask* mask) {
    mask->pos  = -1;
    mask->data = (u8)0;
}

void bitMaskAdd(BitMask* mask, u8 val) {
    int i;
    u8  temp = (u8)(val > 0 ? 1 : 0);

    mask->pos++;

    i = sizeof(mask->data) * BITS_IN_BYTE - 1 - mask->pos;
    temp <<= i;
    mask->data |= temp;
}

u8 bitMaskGet(BitMask* mask, int i) {
    u8  temp = (u8)1;

    i = sizeof(mask->data) * BITS_IN_BYTE - 1 - i;
    temp <<= i;
    temp &= mask->data;

    return (u8)(temp > (u8)0 ? 1 : 0);
}

u8 bitMaskPop(BitMask* mask) {
    u8 res = bitMaskGet(mask, sizeof(mask->data) * BITS_IN_BYTE - 1 - mask->pos);

    mask->pos--;
    return res;
}

int bitMaskIsFull(BitMask* mask) {
    return mask->pos + 1 >= (int)(sizeof(mask->data) * BITS_IN_BYTE);
}

int bitMaskIsEmpty(BitMask* mask) {
    return mask->pos < 0;
}

#undef BITS_IN_BYTE