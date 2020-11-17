#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "dac.h"
#include "adc.h"
#include "usmart.h"

int max(int a, int b)
{
	return a > b ? a : b;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

void clear(char *s, int n)
{
	for (int i = 0; i < n; i++)
	{
		s[i] = '\0';
		s++;
	}
}

int main(void)
{

	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);								//串口初始化为115200
	KEY_Init();										//初始化按键程序
	LED_Init();										//LED端口初始化
	LCD_Init();										//LCD初始化
	usmart_dev.init(72);							//初始化USMART
	Adc_Init();										//ADC初始化
	Dac1_Init();									//DAC初始化

	POINT_COLOR = RED; //设置字体为红色
	LCD_ShowString(60, 130, 200, 16, 16, "WK_UP: ON/OFF");
	//显示提示信息
	POINT_COLOR = BLUE; //设置字体为蓝色
	LCD_ShowString(60, 150, 200, 16, 16, "DAC VAL: ");
	LCD_ShowString(60, 170, 200, 16, 16, "DAC VOL: 0.000V");
	LCD_ShowString(60, 190, 200, 16, 16, "ADC VOL: 0.000V");
	LCD_ShowString(60, 210, 200, 16, 16, "Current: 0.00mA");
	// (1.23/R1 + (1.23-DAC_V)/R3) * R2 + 1.23 == 5
	// DAC_V = DATA * (3.3 / 4096)
	// DATA = DAC_V / 3.3 * 4096
	u16 dacval = (u16)(1.317576 / 3.3 * 4096);
	u8 dacval_step = 4;
	DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //初始值

	u16 adc_r; // measured by /ou/
	u8 t = 0;
	u8 key;
	while (1)
	{
		t = (t + 1) % 10;
		key = KEY_Scan(0);
		if (key == WK_UP)
		{
			PBout(5) = !PBout(5); // turn on or off XL1509
		}
		else if (key == KEY0_PRES)
		{
			dacval = min(4000, dacval + dacval_step);
			DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //设置DAC值
		}
		else if (key == KEY1_PRES)
		{
			dacval = max(400, dacval - dacval_step);
			DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //设置DAC值
		}
		if (t == 0 || key == KEY0_PRES || key == KEY1_PRES || key == WKUP_PRES) //按键按下了,或者定时时间到了
		{
			u16 tmp = DAC_GetDataOutputValue(DAC_Channel_1); //读取前面设置DAC的值
			LCD_ShowxNum(124, 150, tmp, 4, 16, 0);			 //显示DAC寄存器值

			char s[25] = {0};
			sprintf(s, "DAC VOL: %5.3lfV", tmp * (3.3 / 4096)); //显示DAC电压值
			LCD_ShowString(60, 170, 200, 16, 16, (u8 *)s);
			clear(s, 25);

			tmp = Get_Adc_Average(ADC_Channel_1, 10); //得到ADC转换值
			double adc_v = tmp * (3.3 / 4096);
			sprintf(s, "ADC VOL: %5.3lfV", adc_v); //显示ADC电压值
			LCD_ShowString(60, 190, 200, 16, 16, (u8 *)s);
			clear(s, 25);

			sprintf(s, "Current: %6.2lfmA", adc_v / adc_r * 1000);
			LCD_ShowString(60, 210, 200, 16, 16, (u8 *)s);
			clear(s, 25);
		}
		delay_ms(10);
	}
}
