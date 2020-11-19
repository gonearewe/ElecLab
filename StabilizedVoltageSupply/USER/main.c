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

	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200
	KEY_Init();										//��ʼ����������
	LED_Init();										//LED�˿ڳ�ʼ��
	LCD_Init();										//LCD��ʼ��
	usmart_dev.init(72);							//��ʼ��USMART
	Adc_Init();										//ADC��ʼ��
	Dac1_Init();									//DAC��ʼ��

	POINT_COLOR = RED; //��������Ϊ��ɫ
	LCD_ShowString(60, 130, 200, 16, 16, "WK_UP: ON/OFF");
	//��ʾ��ʾ��Ϣ
	POINT_COLOR = BLUE; //��������Ϊ��ɫ
	LCD_ShowString(60, 150, 200, 16, 16, "DAC VAL: ");
	LCD_ShowString(60, 170, 200, 16, 16, "DAC VOL: 0.000V");
	LCD_ShowString(60, 190, 200, 16, 16, "ADC VOL: 0.000V");
	LCD_ShowString(60, 210, 200, 16, 16, "Current: 0.00mA");
	// (1.23/R1 + (1.23-DAC_V)/R3) * R2 + 1.23 == 5
	// DAC_V = DATA * (3.3 / 4096)
	// DATA = DAC_V / 3.3 * 4096
	u16 dacval = (u16)(2.603 / 3.3 * 4096); // 3231
	u8 dacval_step = 10;
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
			u16 tmp = DAC_GetDataOutputValue(DAC_Channel_1); //��ȡǰ������DAC��ֵ
			LCD_ShowxNum(124, 150, tmp, 4, 16, 0);			 //��ʾDAC�Ĵ���ֵ

			char display_buf[25] = {0};
			sprintf(display_buf, "DAC VOL: %5.3lfV", tmp * (3.3 / 4096)); //��ʾDAC��ѹֵ
			LCD_ShowString(60, 170, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			tmp = Get_Adc_Average(ADC_Channel_1, 10);		  //�õ�ADCת��ֵ
			double adc_v = tmp * (3.3 / 4096) * 1000;		  // measured by mV
			sprintf(display_buf, "ADC VOL: %5.3lfmV", adc_v); //��ʾADC��ѹֵ
			LCD_ShowString(60, 190, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);

			sprintf(display_buf, "Current: %6.2lfmA", adc_v / adc_r);
			LCD_ShowString(60, 210, 200, 16, 16, (u8 *)display_buf);
			clear(display_buf, 25);
		}
		delay_ms(10);
	}
}
