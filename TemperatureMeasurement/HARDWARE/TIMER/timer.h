#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#include "rtc.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//定时器 驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

// 历史报警记录
typedef struct Record
{
    float temp_limit;
    float actual_temp;
    _calendar_obj time;
} Record;

extern volatile Record records[5];
extern volatile int records_len;
extern volatile float TEMPERATURE_UPPER_LIMIT;
void TIM3_Int_Init(u16 arr, u16 psc);

#endif
