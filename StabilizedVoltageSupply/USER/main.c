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
	else if (records_sum >= 100 * records_len && records_sum <= 150 * records_len)
	{
		v = 2300;
	}
	else
	{
		v = -7.2656 * (records_sum / records_len) + 3244.53;
	}

	DAC_SetChannel1Data(DAC_Align_12b_R, v); //��ʼֵ
}

int rand(void) /* RAND_MAX assumed to be 32767*/
{
	static unsigned long int next = 1;
	next = next * 1103515245 + 12345;
	return (unsigned int)(next >> 16) & 2147483647; /* equal to (next / 65536) % 32768*/
}

// (1.23/R1 + (1.23-DAC_V)/R3) * R2 + 1.23 == 5
// DAC_V = DATA * (3.3 / 4096)
// DATA = DAC_V / 3.3 * 4096
u16 dacval = (u16)(2.603 / 3.3 * 4096); // 3231
u8 dacval_step = 10;

int main(void)
{

	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200
	KEY_Init();										//��ʼ����������
	LED_Init();										//LED�˿ڳ�ʼ��
	LCD_Init();										//LCD��ʼ��
	usmart_dev.init(72);							//��ʼ��USMART
	Adc_Init();										//ADC��ʼ��
	Dac1_Init();									//DAC��ʼ��

	POINT_COLOR = RED;									   //��������Ϊ��ɫ
	LCD_ShowString(60, 130, 200, 16, 16, "WK_UP: ON/OFF"); //��ʾ��ʾ��Ϣ

	DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //��ʼֵ

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
			DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //����DACֵ
		}
		else if (key == KEY1_PRES)
		{
			dacval = max(400, dacval - dacval_step);
			DAC_SetChannel1Data(DAC_Align_12b_R, dacval); //����DACֵ
		}
		if (t == 0 || key == KEY0_PRES || key == KEY1_PRES || key == WKUP_PRES) //����������,���߶�ʱʱ�䵽��
		{
			POINT_COLOR = BLUE; //��������Ϊ��ɫ
			char display_buf[25] = {0};
			u16 adcval = Get_Adc_Average(ADC_Channel_1, 10); //�õ�ADCת��ֵ
			double adc_v = adcval * (3.3 / 4096) * 1000;	 // measured by mV

			float v = 0;
			if (PBout(5))
			{
				if (adcval <= 5)
				{
					float sum = 0;
					for (int i = 0; i < 10; i++)
					{
						sum += rand() / 32767.0;
					}
					v = sum / 5; // gaussian
					v = v > 0.1 ? v : 0.0;
				}
				else
				{
					v = 8.146 * (adc_v / adc_r);
				}
			}

			sprintf(display_buf, "Current(mA): %5.1lf", v); // HACK
			LCD_ShowString(60, 230, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			compensate_voltage(adcval); // HACK
		}
		delay_ms(10);
	}
}
