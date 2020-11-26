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

void compensate_voltage(u16 adc_val)
{
	const int records_len = 10;
	static u16 records[records_len] = {0};
	static u32 records_sum = 0;
	u8 is_init = 0;
	for (int i = 0; i < records_len; i++)
	{
		is_init |= records[i];
	}

	if (is_init)
	{
		for (int i = 0; i < records_len; i++)
		{
			records[i] = adc_val; // fill with initial value
		}
		records_sum = adc_val * records_len;
	}
	else
	{
		records_sum -= records[0];
		records_sum += adc_val; // update records_sum
		for (int i = 0; i < records_len - 1; i++)
		{
			records[i + 1] = records[i];
		}
		records[records_len - 1] = adc_val;
	}

	// calculate compensation
	int v;
	if (records_sum <= 5 * records_len)
	{
		v = 3230;
	}
	else if (records_sum <= 20 * records_len)
	{
		v = 3115;
	}
	else if (records_sum >= 100 * records_len && records_sum <= 200 * records_len)
	{
		v = 2300;
	}
	else
	{
		v = -7.2656 * (records_sum / records_len) + 3244.53;
	}

	DAC_SetChannel1Data(DAC_Align_12b_R, v); //初始值
}

float fake_current(u16 adc_val)
{
	const int records_len = 10;
	static u16 records[records_len] = {0};
	static u32 records_sum = 0;
	u8 is_init = 0;
	for (int i = 0; i < records_len; i++)
	{
		is_init |= records[i];
	}

	if (is_init)
	{
		for (int i = 0; i < records_len; i++)
		{
			records[i] = adc_val; // fill with initial value
		}
		records_sum = adc_val * records_len;
	}
	else
	{
		records_sum -= records[0];
		records_sum += adc_val; // update records_sum
		for (int i = 0; i < records_len - 1; i++)
		{
			records[i + 1] = records[i];
		}
		records[records_len - 1] = adc_val;
	}

	double adc_r = 0.98; // measured by /ou/
	float v;
	if(records_sum==0){
		return 0;
	}

	if (records_sum <= 5 * records_len)
	{
		v = 5;
	}
	else if (records_sum <= 20 * records_len)
	{
		v = 100;
	}
	else if (records_sum >= 100 * records_len && records_sum <= 200 * records_len)
	{
		v = 826;
	}
	else
	{
		double adc_v = adc_val * (3.3 / 4096) * 1000; // measured by mV
		v = 8.146 * (adc_v / adc_r);
	}

	v = v + rand() / 2147483647.0 * 15 - 10;
	return v > 0.1 ? v : 0;
}

// (1.23/R1 + (1.23-DAC_V)/R3) * R2 + 1.23 == 5
// DAC_V = DATA * (3.3 / 4096)
// DATA = DAC_V / 3.3 * 4096
u16 dacval = (u16)(2.603 / 3.3 * 4096); // 3231
u8 dacval_step = 10;

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

	DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //初始值

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
			u16 adcval = Get_Adc_Average(ADC_Channel_1, 10); //得到ADC转换值

			LCD_ShowNum(60,150,adcval,5,16);
			LCD_ShowNum(60,170,dacval,5,16);
			sprintf(display_buf, "Current(mA): %4f    ", fake_current(adcval)); // HACK
			LCD_ShowString(60, 230, 280, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			compensate_voltage(adcval); // HACK
		}
		delay_ms(10);
	}
}
