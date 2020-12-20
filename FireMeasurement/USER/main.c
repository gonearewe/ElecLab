#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "dac.h"
#include "timer.h"
#include "fire.h"

void fire_draw_rows(u16 row_start, u16 row_end, const u16 layout[][16], u16 color)
{
	const int x = 10, y = 5, len = 16;
	for (int i = row_end; i >= row_start; i--)
	{
		for (int j = 0, c = layout[i][j], l = layout[i][j + 1]; c != 0 && j + 1 < len; j += 2, c = layout[i][j], l = layout[i][j + 1])
		{
			for (int k = c; k < c + l; k++)
			{
				LCD_Fast_DrawPoint(x + k, y + i, color);
			}
		}
	}
}

// stage: 0, 1, 2, ... , 9
void fire_draw(u8 stage)
{
	static u16 last_row = FIRE_LAYOUT_LEN - 1;
	u16 row = (9 - stage) * (FIRE_LAYOUT_LEN / 10);
	short diff = row - last_row;
	if (diff > 0)
	{
		// clear fire
		fire_draw_rows(last_row, row, outer_fill, WHITE);
		fire_draw_rows(last_row, row, inner_fill, WHITE);
	}
	else if (diff < 0)
	{
		fire_draw_rows(row, last_row, inner_fill, YELLOW);
		fire_draw_rows(row, last_row, outer_fill, RED);
	}
	last_row = row;
}

int main(void)
{
	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);								//串口初始化为115200
	LED_Init();										//LED端口初始化
	LCD_Init();
	TIM3_Int_Init(99, 7199); // 100Hz
	Adc_Init();				 //ADC初始化
	Dac1_Init();

	POINT_COLOR = BLUE; //设置字体为蓝色
						//	LCD_ShowString(60, 130, 200, 16, 16, "ADC_CH0_VAL:");
						//	LCD_ShowString(60, 150, 200, 16, 16, "ADC_CH0_VOL:0.000V");

	fire_draw_rows(0, 499, outline, RED);
	u16 adcx;
	while (1)
	{
		adcx = Get_Adc_Average(ADC_Channel_1, 30);
		LCD_ShowxNum(5, 2, adcx, 4, 16, 0); //显示ADC的值

		const u16 seperators[11]={4095,1990,1410,1137,978,870,784,721,668,635,1};
		for(int i = 0;i<10;i++){
			if(adcx<seperators[i]&&adcx>=seperators[i+1]){
				LCD_ShowxNum(10,20,i+1,2,24,0);
				LCD_ShowChar(50,20,'M',24,0);
				FIRE_STAGE = 9-i;
				fire_draw(FIRE_STAGE);
				break;
			}
		}
	}
}
