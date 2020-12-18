#ifndef __FIRE_H_
#define __FIRE_H_

#include "stm32f10x.h"

#define FIRE_LAYOUT_LEN 500

extern const u16 outline[FIRE_LAYOUT_LEN][16];
extern const u16 outer_fill[FIRE_LAYOUT_LEN][16];
extern const u16 inner_fill[FIRE_LAYOUT_LEN][16];

#endif
