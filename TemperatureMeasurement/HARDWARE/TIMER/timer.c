#include "timer.h"
#include "led.h"
#include "rtc.h"
#include "lcd.h"
#include "max31865.h"
#include "stdio.h"
// volatile TEMPERATURE_WARN_FLAG

//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;						//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //��ʼ��NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE); //ʹ��TIMx
}

void display_time(void)
{
	POINT_COLOR = BLUE;

	char buf[35] = {0};
	sprintf(buf, "%04d-%02d-%02d", calendar.w_year, calendar.w_month, calendar.w_date);
	LCD_ShowString(60, 130, 200, 16, 16, (u8 *)buf);
	for (int i = 0; i < 35; i++)
	{
		buf[i] = '\0';
	}

	sprintf(buf, "%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);
	LCD_ShowString(60, 162, 200, 16, 16, (u8 *)buf);

	const char *week_table[7] = {"Sunday   ", "Monday   ", "Tuesday  ", "Wednesday", "Thursday ", "Friday   ", "Saturday "};
	LCD_ShowString(60, 148, 200, 16, 16, (u8 *)week_table[calendar.week]);
}

//��ʱ��3�жϷ������
void TIM3_IRQHandler(void) //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���TIM3�����жϷ������
	{
		static u8 warn_period = 0;
		static bool warn_permit = true;
		float temp = temperature(100, 430);
		if (temp < TEMPERATURE_UPPER_LIMIT && warn_period == 0)
		{
			warn_permit = true;
		}
		if (temp > TEMPERATURE_UPPER_LIMIT && warn_period == 0 && warn_permit)
		{
			static u8 records_id = 0;
			records[records_id].temp_limit = TEMPERATURE_UPPER_LIMIT;
			records[records_id].actual_temp = temp;
			records[records_id].time = calendar;
			records_id = (records_id + 1) % 5;
			records_len = records_len + 1 > 5 ? 5 : records_len + 1;

			warn_period = 10;	// 10*500 ms == 5 s
			LED1 = 1;			 // �̵�Ϩ��ͬʱ��������
			LED0 = 0;			 // �������
			warn_permit = false; // to avoid continuous warnning
		}
		if (warn_period != 0)
		{
			warn_period--;
		}
		else
		{ // �ָ�����״̬
			LED1 = 0;
			LED0 = 1;
		}

		char buf[35] = {0};
		sprintf(buf, "Limit(Celsius): %d       ", (int)TEMPERATURE_UPPER_LIMIT);
		LCD_ShowString(60, 60, 400, 16, 16, (u8 *)buf);
		for (int i = 0; i < 35; i++)
		{
			buf[i] = '\0';
		}

		sprintf(buf, "Temperature(Celsius): %d       ", (int)temp);
		LCD_ShowString(60, 90, 400, 16, 16, (u8 *)buf);
		display_time();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //���TIMx�����жϱ�־
	}
}
