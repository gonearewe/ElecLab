#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#include "rtc.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��ʱ�� ��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

// ��ʷ������¼
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
