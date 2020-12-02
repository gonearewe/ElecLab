#include "timer.h"
#include "led.h"
#include "rtc.h"
#include "lcd.h"
#include "max31865.h"
#include "stdbool.h"
// volatile TEMPERATURE_WARN_FLAG

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr;						//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				//根据指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);							  //初始化NVIC寄存器

	TIM_Cmd(TIM3, ENABLE); //使能TIMx
}

void display_time(void)
{
	POINT_COLOR = BLUE;

	static bool isInit = true;
	if (isInit)
	{
		LCD_ShowString(60, 130, 200, 16, 16, "    -  -  ");
		LCD_ShowString(60, 162, 200, 16, 16, "  :  :  ");
		isInit = false;
	}

	LCD_ShowNum(60, 130, calendar.w_year, 4, 16);
	LCD_ShowNum(100, 130, calendar.w_month, 2, 16);
	LCD_ShowNum(124, 130, calendar.w_date, 2, 16);
	switch (calendar.week)
	{
	case 0:
		LCD_ShowString(60, 148, 200, 16, 16, "Sunday   ");
		break;
	case 1:
		LCD_ShowString(60, 148, 200, 16, 16, "Monday   ");
		break;
	case 2:
		LCD_ShowString(60, 148, 200, 16, 16, "Tuesday  ");
		break;
	case 3:
		LCD_ShowString(60, 148, 200, 16, 16, "Wednesday");
		break;
	case 4:
		LCD_ShowString(60, 148, 200, 16, 16, "Thursday ");
		break;
	case 5:
		LCD_ShowString(60, 148, 200, 16, 16, "Friday   ");
		break;
	case 6:
		LCD_ShowString(60, 148, 200, 16, 16, "Saturday ");
		break;
	}
	LCD_ShowNum(60, 162, calendar.hour, 2, 16);
	LCD_ShowNum(84, 162, calendar.min, 2, 16);
	LCD_ShowNum(108, 162, calendar.sec, 2, 16);
}

//定时器3中断服务程序
void TIM3_IRQHandler(void) //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查TIM3更新中断发生与否
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
			records[records_len].temp_limit = TEMPERATURE_UPPER_LIMIT;
			records[records_len].actual_temp = temp;
			records[records_len].time = calendar;
			records_id = (records_id + 1) % 5;
			records_len = records_len + 1 > 5 ? 5 : records_len + 1;

			warn_period = 10;	 // 10*500 ms == 5 s
			LED1 = 1;			 // 绿灯熄灭同时蜂鸣器响
			LED0 = 0;			 // 红灯亮起
			warn_permit = false; // to avoid continuous warnning
		}
		if (warn_period != 0)
		{
			warn_period--;
		}
		else
		{ // 恢复正常状态
			LED1 = 0;
			LED0 = 1;
		}

		char buf[35] = {0};
		sprintf(buf, "Limit(Celsius): %3.2f", TEMPERATURE_UPPER_LIMIT);
		LCD_ShowString(60, 60, 300, 16, 16, (u8 *)buf);
		for (int i = 0; i < 35; i++)
		{
			buf[i] = '\0';
		}

		sprintf(buf, "Temperature(Celsius): %3.0f", temp);
		LCD_ShowString(60, 90, 400, 16, 16, (u8 *)buf);
		display_time();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除TIMx更新中断标志
	}
}
