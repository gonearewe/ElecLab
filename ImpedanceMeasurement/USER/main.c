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
#include "math.h"

float x[512];
void extract_pattern(float *dc, float *ac, unsigned long freq)
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
		LCD_DrawLine(10, i + 25, 10 + x[i] * (280 / 3.3), i + 25);
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

	*dc = (x[0] * 1000 + 0.5) / 100;
	*ac = (max * 1000 + 0.5) / 100;
	LCD_ShowNum(0, 0, freq / 1000, 3, 16);
	LCD_ShowNum(50, 0, *dc, 7, 16);
	LCD_ShowNum(120, 0, *ac, 7, 16);
}

float table[][4] = {
	{1800, 229, 1800, 2},
	{1800, 53, 1800, 1},
	{140, 57, 140, 61},
	{828, 334, 834, 355},
	{296, 120, 300, 130},
	{2, 0, 66, 51},
	{4, 2, 127, 96},
	{1800, 217, 1800, 61},
	{1800, 350, 1800, 355},
	{1800, 224, 1800, 130},
	{1800, 76, 1800, 61},
	{1800, 330, 1800, 352},
	{1800, 130, 1800, 130},

	{142, 57, 162, 99},
	{850, 328, 835, 353},
	{308, 120, 308, 155},
	{144, 57, 208, 140},
	{850, 330, 840, 360},
	{310, 120, 330, 190},

	{1800, 222, 1800, 102},
	{1800, 225, 1800, 190},
	{1800, 52, 1800, 104},
	{1800, 51, 1800, 190},
	{139, 56, 140, 2},
	{840, 193, 836, 2},
	{300, 106, 300, 2},
	{140, 38, 140, 1},
	{835, 53, 835, 1},
	{300, 49, 300, 1},

	{1, 0, 33, 25},
	{1, 0, 65, 49},
	{1, 0, 51, 40},
	{2, 1, 38, 29},
	{2, 1, 114, 86},
	{2, 1, 70, 53},

	{2, 0, 2, 0},
	{2, 0, 2, 0},
	{3, 1, 4, 1},
	{3, 1, 3, 0}

};

char *s[39] = {
	"C1     ",
	"C2     ",
	"R1     ",
	"R2     ",
	"R3     ",
	"L1     ",
	"L2     ",
	"R1+C1  ",
	"R2+C1  ",
	"R3+C1  ",
	"R1+C2  ",
	"R2+C2  ",
	"R3+C2  ",
	"R1+L1  ",
	"R2+L1  ",
	"R3+L1  ",
	"R1+L2  ",
	"R2+L2  ",
	"R3+L2  ",
	"L1+C1  ",
	"L2+C1  ",
	"L1+C2  ",
	"L2+C2  ",
	"R1//C1",
	"R2//C1",
	"R3//C1",
	"R1//C2",
	"R2//C2",
	"R3//C2",
	"R1//L1",
	"R2//L1",
	"R3//L1",
	"R1//L2",
	"R2//L2",
	"R3//L2",
	"C1//L1",
	"C2//L1",
	"C1//L2",
	"C2//L2",
};

// use similarity to determine which kind of impedance it is
u8 which(float x1, float x2, float x3, float x4)
{
	float min = 100000;
	u8 min_i = 0;
	for (int i = 0; i < 39; i++)
	{
		// 余弦相似度
		// float x = table[i][0] * x1 + table[i][1] * x2 + table[i][2] * x3 + table[i][3] * x4;
		// x /= sqrt(table[i][0] * table[i][0] + table[i][1] * table[i][1] + table[i][2] * table[i][2] + table[i][3] * table[i][3]);
		// x /= sqrt(x1 * x1 + x2 * x2 + x3 * x3 + x4 * x4);

		// 欧式距离
		float x = sqrt(pow(table[i][0] - x1, 2) + pow(table[i][1] - x2, 2) + pow(table[i][2] - x3, 2) + pow(table[i][3] - x4, 2));
		if (x < min)
		{
			min = x;
			min_i = i;
		}
	}
	return min_i;
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
		float data[4];
		extract_pattern(&data[0], &data[1], 1000);
		extract_pattern(&data[2], &data[3], 100000);
		u8 i = which(data[0], data[1], data[2], data[3]);
		LCD_ShowString(200, 0, 100, 8, 24, (u8 *)s[i]);
		delay_ms(500);
	}
}
