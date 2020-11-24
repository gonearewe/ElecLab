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

void clear(char *display_buf, int n)
{
	for (int i = 0; i < n; i++)
	{
		display_buf[i] = '\0';
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

	POINT_COLOR = RED;									   //设置字体为红色
	LCD_ShowString(60, 130, 200, 16, 16, "WK_UP: ON/OFF"); //显示提示信息

	// (1.23/R1 + (1.23-DAC_V)/R3) * R2 + 1.23 == 5
	// DAC_V = DATA * (3.3 / 4096)
	// DATA = DAC_V / 3.3 * 4096
	u16 dacval = (u16)(2.603 / 3.3 * 4096); // 3231
	u8 dacval_step = 10;
	DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //初始值

	double adc_r = 0.98; // measured by /ou/
	u8 t = 0;
	u8 key;
	while (1)
	{
		t = (t + 1) % 10;
		key = KEY_Scan(0);
		if (key == WKUP_PRES)
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
			POINT_COLOR = BLUE; //设置字体为蓝色
			char display_buf[25] = {0};

			u16 tmp = DAC_GetDataOutputValue(DAC_Channel_1); //读取前面设置DAC的值
			sprintf(display_buf, "DAC VAL: %d", tmp);
			LCD_ShowString(60, 150, 200, 16, 16, (u8 *)display_buf); //显示DAC寄存器值
			clear(display_buf, 25);

			sprintf(display_buf, "DAC VOL: %5.3lfV", tmp * (3.3 / 4096)); //显示DAC电压值
			LCD_ShowString(60, 170, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			tmp = Get_Adc_Average(ADC_Channel_1, 10);	//得到ADC转换值
			sprintf(display_buf, "ADC VAL: %d  ", tmp); //显示ADC电压值
			LCD_ShowString(60, 190, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			double adc_v = tmp * (3.3 / 4096) * 1000;		  // measured by mV
			sprintf(display_buf, "ADC VOL: %5.3lfmV", adc_v); //显示ADC电压值
			LCD_ShowString(60, 210, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			POINT_COLOR = MAGENTA;
			sprintf(display_buf, "Current: %6.2lfmA", adc_v / adc_r);
			LCD_ShowString(60, 230, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);
		}
		delay_ms(10);
	}
}
