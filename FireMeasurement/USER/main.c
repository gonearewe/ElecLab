#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "timer.h"
#include "fire.h"

/************************************************
 ALIENTEK��ӢSTM32������ʵ��17
 ADC ʵ��   
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

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
	u16 adcx;
	float temp;
	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200
	LED_Init();										//LED�˿ڳ�ʼ��
	LCD_Init();
	TIM3_Int_Init(99, 7199); // 100Hz
	Adc_Init();				 //ADC��ʼ��

	POINT_COLOR = BLUE; //��������Ϊ��ɫ
						//	LCD_ShowString(60, 130, 200, 16, 16, "ADC_CH0_VAL:");
						//	LCD_ShowString(60, 150, 200, 16, 16, "ADC_CH0_VOL:0.000V");

	fire_draw_rows(0, 499, outline, RED);
	while (1)
	{
		for (int i = 0; i < 10; i++)
		{
			fire_draw(i);
			delay_ms(100);
		}
		for (int i = 9; i >= 0; i--)
		{
			fire_draw(i);
			delay_ms(100);
		}
	}
	//	{
	//		adcx = Get_Adc_Average(ADC_Channel_1, 10);
	//		LCD_ShowxNum(156, 130, adcx, 4, 16, 0); //��ʾADC��ֵ
	//		temp = (float)adcx * (3.3 / 4096);
	//		adcx = temp;
	//		LCD_ShowxNum(156, 150, adcx, 1, 16, 0); //��ʾ��ѹֵ
	//		temp -= adcx;
	//		temp *= 1000;
	//		LCD_ShowxNum(172, 150, temp, 3, 16, 0X80);
	//		LED0 = !LED0;
	//		delay_ms(250);
	//	}
}
