#include "timer.h"
#include "led.h"
#include "rtc.h"
#include "lcd.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//定时器 驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

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
	POINT_COLOR = BLUE; //设置字体为蓝色

	static u8 isInit = 1;
	if (isInit)
	{
		LCD_ShowString(60, 130, 200, 16, 16, "    -  -  ");
		LCD_ShowString(60, 162, 200, 16, 16, "  :  :  ");
		isInit = 0;
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
		display_time();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除TIMx更新中断标志
	}
}
