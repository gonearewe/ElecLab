#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "24cxx.h"
#include "w25qxx.h"
#include "touch.h"
#include "ad9834.h"
#include "adc.h"
#include "timer.h"
#include "fft.h"

float x[512];
void extract_pattern(u32 *dc, u32 *ac, unsigned long freq)
{
	AD9834_Set_Freq(FREQ_0, freq);
	delay_ms(100); // wait until it's steady

	for (int i = 0; i < 512; i++)
	{
		x[i] = Get_Adc(ADC_Channel_1) * (3.3 / 4096);
		//delay_ms(2);
	}

	// draw diagram
	LCD_Fill(10, 20, 320, 500, WHITE);
	for (int i = 0; i < 480; i++)
	{
		LCD_DrawLine(10, i + 20, 10 + x[i] * (280 / 3.3), i + 20);
	}

	fft(x, 512);
	float max = 0;
	for (int i = 1; i < 512 / 2; i++)
	{
		if (x[i] > max)
		{
			max = x[i];
		}
	}

	*dc=(u32)(x[0] * 1000 + 0.5);
	*ac=(u32)(max * 1000 + 0.5);
	LCD_ShowNum(0, 0, *dc, 8, 16);
	LCD_ShowNum(60, 0,*ac , 8, 16);
}

int main(void)
{
	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);								//串口初始化为115200
	Adc_Init();
	LED_Init(); //LED端口初始化
	LCD_Init();
	KEY_Init();
	AD9834_Init();
	AD9834_Select_Wave(Sine_Wave); //Sine_Wave  Square_Wave  Triangle_Wave

	while (1)
	{
		u32 *dc, *ac;
		extract_pattern(dc,ac,1000);
		// TODO

		extract_pattern(dc,ac,100000);
		if(*dc<=12&&*ac<=8){
			LCD_ShowString(150,0,100,8,16,"L1");
		}else if(*dc<=15&&*ac<=12)
		{
			LCD_ShowString(150,0,100,8,16,"L2");
		}
		
		// LCD_ShowNum(50,0,Get_Adc_Average(ADC_Channel_1,250),4,16);
	}
}
